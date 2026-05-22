/**
 * BLE 蓝牙通信服务
 * 
 * 与 ESP32-C3 数控电源控制板通过 BLE 通信
 * ESP32-C3 端需运行 BLE UART (NUS) 服务
 * 
 * 通信协议 (JSON over BLE):
 *   请求: {"cmd":"get_data"}           → 获取电源数据
 *   响应: {"V_out":12.0,"I_out":5.0,...}
 *   
 *   请求: {"cmd":"set","V_set":12.0}   → 设定电压
 *   请求: {"cmd":"set","I_set":10.0}   → 设定电流
 *   请求: {"cmd":"set","power":1}      → 开机 (1) / 关机 (0)
 *   请求: {"cmd":"get_info"}           → 获取设备信息
 */

// BLE 服务 UUID (与 ESP32-C3 端约定)
const BLE_SERVICE_UUID = '6E400001-B5A3-F393-E0A9-E50E24DCCA9E'
const BLE_TX_UUID = '6E400002-B5A3-F393-E0A9-E50E24DCCA9E' // ESP32 → App (通知)
const BLE_RX_UUID = '6E400003-B5A3-F393-E0A9-E50E24DCCA9E' // App → ESP32 (写入)

class BleService {
  constructor() {
    this._deviceId = ''
    this._serviceId = ''
    this._txCharId = ''
    this._rxCharId = ''
    this._connected = false
    this._onDataCallback = null
    this._onStatusCallback = null
    this._receiveBuffer = ''
    this._initialized = false  // 防止重复初始化
    this._connectionStateChangeRegistered = false  // 防止重复注册连接状态监听
    this._lastData = null  // 缓存最后一条数据，用于新页面注册时立即回掉
    this._deviceInfo = null  // 缓存设备信息（连接成功后自动获取一次，固定不变）

    // ---------- 累计电能计算 (App 端积分, 不依赖 PMBus E_in/E_out 寄存器) ----------
    this._energyData = {
      E_out: 0,        // 累计输出电能 (Wh)
      E_in: 0,         // 累计输入电能 (Wh)
      lastW_out: 0,    // 上次输出功率 (W)
      lastW_in: 0,     // 上次输入功率 (W)
      lastTime: 0,     // 上次更新时间戳 (ms)
      accumulated: false // 是否已累积过 (首次数据只记录不累积)
    }
  }

  /**
   * 获取当前连接状态
   */
  get connected() {
    return this._connected
  }

  /**
   * 初始化蓝牙模块（全局只执行一次）
   */
  init() {
    if (this._initialized) {
      console.log('[BLE] 蓝牙适配器已初始化，跳过')
      return
    }
    this._initialized = true

    // 全局注册连接状态变化监听（只注册一次，避免 uni.reLaunch 后丢失）
    if (!this._connectionStateChangeRegistered) {
      this._connectionStateChangeRegistered = true
      uni.onBLEConnectionStateChange((res) => {
        console.log('[BLE] 连接状态变化:', JSON.stringify(res))
        this._connected = res.connected
        if (!res.connected) {
          console.warn('[BLE] 连接已断开，清空连接参数')
          this._deviceId = ''
          this._serviceId = ''
          this._txCharId = ''
          this._rxCharId = ''
          this._receiveBuffer = ''
          this._lastData = null  // 断开连接时清除缓存数据
          // 断开连接时重置累计电能（重新连接后重新开始累积）
          this._resetEnergy()
          this._notifyStatus('已断开')
        } else {
          this._notifyStatus('已连接')
        }
      })
    }

    uni.openBluetoothAdapter({
      success: () => {
        console.log('[BLE] 蓝牙适配器初始化成功')
        this._notifyStatus('蓝牙就绪')
      },
      fail: (err) => {
        console.error('[BLE] 蓝牙适配器初始化失败:', err)
        this._notifyStatus('蓝牙初始化失败，请检查蓝牙权限')
      }
    })
  }

  /**
   * 开始扫描 BLE 设备
   * @param {Function} onFound 发现设备回调 (device)
   */
  startScan(onFound) {
    // #ifdef APP-PLUS
    uni.startBluetoothDevicesDiscovery({
      allowDuplicatesKey: true,
      success: () => {
        console.log('[BLE] 开始扫描设备')
        this._notifyStatus('正在扫描...')
      },
      fail: (err) => {
        console.error('[BLE] 扫描失败:', err)
        this._notifyStatus('扫描失败')
      }
    })

    // 监听发现设备
    uni.onBluetoothDeviceFound((res) => {
      const devices = res.devices
      for (const device of devices) {
        // 调试日志: 打印每个发现的设备
        console.log('[BLE] 发现设备:', JSON.stringify({
          name: device.name,
          id: device.deviceId,
          RSSI: device.RSSI,
          services: device.advertisServiceUUIDs
        }))

        // 通过服务 UUID 过滤 NUS 设备
        const hasNusService = device.advertisServiceUUIDs &&
          device.advertisServiceUUIDs.some(uuid =>
            uuid.toUpperCase() === BLE_SERVICE_UUID
          )

        // 只显示匹配 NUS 服务 UUID 的设备 (Dell-PSU-Controller)
        if (hasNusService) {
          onFound && onFound(device)
        }
      }
    })
    // #endif
  }

  /**
   * 停止扫描
   */
  stopScan() {
    // #ifdef APP-PLUS
    uni.stopBluetoothDevicesDiscovery({
      success: () => {
        console.log('[BLE] 停止扫描')
      }
    })
    // #endif
  }

  /**
   * 连接 BLE 设备
   * @param {string} deviceId 设备 ID
   */
  connect(deviceId) {
    return new Promise((resolve, reject) => {
      this._deviceId = deviceId

      let resolved = false

      // 设置连接超时 (15秒)
      const timeout = setTimeout(() => {
        if (!resolved) {
          console.error('[BLE] 连接超时:', deviceId)
          this._notifyStatus('连接超时')
          this._connected = false
          reject(new Error('连接超时'))
        }
      }, 15000)

      // 发起连接 (连接状态变化由 init() 中注册的全局监听器处理)
      uni.createBLEConnection({
        deviceId,
        timeout: 10000,
        success: () => {
          console.log('[BLE] 连接请求成功:', deviceId)
          this._notifyStatus('连接成功，发现服务...')
          this._connected = true
          // 等待 2 秒确保连接稳定，再发现服务
          setTimeout(() => {
            if (!resolved) {
              resolved = true
              clearTimeout(timeout)
              this._discoverServices().then(resolve).catch(reject)
            }
          }, 2000)
        },
        fail: (err) => {
          if (!resolved) {
            resolved = true
            clearTimeout(timeout)
            console.error('[BLE] 连接失败:', err)
            this._notifyStatus('连接失败')
            this._connected = false
            reject(err)
          }
        }
      })
    })
  }

  /**
   * 标准化 UUID: 去除花括号、转小写，便于比较
   */
  _normalizeUuid(uuid) {
    if (!uuid) return ''
    return uuid.replace(/[{}]/g, '').toLowerCase()
  }

  /**
   * 发现服务和特征值
   */
  _discoverServices() {
    return new Promise((resolve, reject) => {
      uni.getBLEDeviceServices({
        deviceId: this._deviceId,
        success: (res) => {
          const services = res.services
          console.log('[BLE] 发现服务列表:', JSON.stringify(services.map(s => s.uuid)))

          // 标准化目标 UUID
          const targetUuid = this._normalizeUuid(BLE_SERVICE_UUID)

          // 查找 NUS 服务 (兼容各种 UUID 格式)
          let nusService = null
          for (const s of services) {
            const normalized = this._normalizeUuid(s.uuid)
            console.log('[BLE]  服务UUID:', s.uuid, '→ 标准化:', normalized)
            if (normalized === targetUuid || normalized.includes(targetUuid) || targetUuid.includes(normalized)) {
              nusService = s
              break
            }
          }

          if (!nusService) {
            // 如果没找到标准 UUID，使用第一个可用服务
            console.warn('[BLE] 未找到标准 NUS 服务，尝试第一个可用服务')
            if (services.length > 0) {
              this._serviceId = services[0].uuid
            } else {
              reject(new Error('未找到任何服务'))
              return
            }
          } else {
            this._serviceId = nusService.uuid
          }

          console.log('[BLE] 使用服务:', this._serviceId)
          this._discoverCharacteristics().then(resolve).catch(reject)
        },
        fail: (err) => {
          console.error('[BLE] 获取服务失败:', err)
          reject(err)
        }
      })
    })
  }

  /**
   * 发现特征值
   */
  _discoverCharacteristics() {
    return new Promise((resolve, reject) => {
      uni.getBLEDeviceCharacteristics({
        deviceId: this._deviceId,
        serviceId: this._serviceId,
        success: (res) => {
          const chars = res.characteristics

          // 标准化目标 UUID (去除花括号、转小写)
          const targetTxUuid = this._normalizeUuid(BLE_TX_UUID)
          const targetRxUuid = this._normalizeUuid(BLE_RX_UUID)

          // 查找 TX (通知) 和 RX (写入) 特征值
          for (const char of chars) {
            const normalized = this._normalizeUuid(char.uuid)
            console.log('[BLE]  特征值UUID:', char.uuid, '→ 标准化:', normalized,
                        'properties:', JSON.stringify(char.properties))
            if (normalized === targetTxUuid) {
              this._txCharId = char.uuid
              console.log('[BLE]  ✓ 匹配 TX 特征值')
            } else if (normalized === targetRxUuid) {
              this._rxCharId = char.uuid
              console.log('[BLE]  ✓ 匹配 RX 特征值')
            }
          }

          // 如果没找到标准 UUID，使用第一个可通知和第一个可写入的特征值
          if (!this._txCharId || !this._rxCharId) {
            console.warn('[BLE] 未通过UUID匹配到特征值，使用属性匹配')
            for (const char of chars) {
              if (!this._txCharId && char.properties.notify) {
                this._txCharId = char.uuid
                console.log('[BLE]  fallback TX:', char.uuid)
              }
              if (!this._rxCharId && char.properties.write) {
                this._rxCharId = char.uuid
                console.log('[BLE]  fallback RX:', char.uuid)
              }
            }
          }

          console.log('[BLE] 最终 TX:', this._txCharId, 'RX:', this._rxCharId)

          if (!this._txCharId || !this._rxCharId) {
            reject(new Error('未找到必要的特征值'))
            return
          }

          // 启用通知
          this._enableNotify().then(resolve).catch(reject)
        },
        fail: (err) => {
          console.error('[BLE] 获取特征值失败:', err)
          reject(err)
        }
      })
    })
  }

  /**
   * 启用通知
   */
  _enableNotify() {
    return new Promise((resolve, reject) => {
      uni.notifyBLECharacteristicValueChange({
        deviceId: this._deviceId,
        serviceId: this._serviceId,
        characteristicId: this._txCharId,
        state: true,
        success: () => {
          console.log('[BLE] 通知已启用')
          // 监听数据
          this._startListening()
          this._notifyStatus('已连接')
          // 连接成功后自动获取一次设备信息（固定不变，缓存后不再重复请求）
          this._requestDeviceInfo()
          resolve()
        },
        fail: (err) => {
          console.error('[BLE] 启用通知失败:', err)
          // 即使通知启用失败，连接本身是成功的，仍然 resolve
          // 这样用户至少可以发送指令，只是收不到通知
          console.warn('[BLE] 通知不可用，仅支持单向写入')
          this._startListening()
          this._notifyStatus('已连接（通知不可用）')
          resolve()
        }
      })
    })
  }

  /**
   * 开始监听数据
   */
  _startListening() {
    uni.onBLECharacteristicValueChange((res) => {
      // 直接将 ArrayBuffer 转为字符串 (兼容 uni-app Android, 避免 TextDecoder 不可用)
      const text = this._arrayBufferToString(res.value)

      // 处理粘包
      this._receiveBuffer += text
      const lines = this._receiveBuffer.split('\n')
      // 最后一个可能不完整，保留到下次
      this._receiveBuffer = lines.pop() || ''

      // 安全保护: 如果缓冲区超过 2KB 还没有遇到 '\n', 说明数据异常, 清空重来
      if (this._receiveBuffer.length > 2048) {
        console.warn('[BLE] 接收缓冲区异常 (超过2KB无换行), 清空重来')
        this._receiveBuffer = ''
        return
      }

      for (const line of lines) {
        const trimmed = line.trim()
        if (trimmed) {
          try {
            const json = JSON.parse(trimmed)
            console.log('[BLE] 收到数据:', json)
            // 缓存设备信息（MFR 数据是固定不变的，连接后只获取一次）
            if (json.MFR_ID) {
              this._deviceInfo = json
            }
            // 注入 App 端自行计算的累计电能（替换 PMBus 不可靠的 E_in/E_out）
            this._injectEnergyData(json)
            // 缓存最后一条数据，用于新页面注册时立即回掉
            this._lastData = json
            this._onDataCallback && this._onDataCallback(json)
          } catch (e) {
            console.warn('[BLE] 解析失败:', trimmed)
          }
        }
      }
    })
  }

  /**
   * 累计电能积分计算
   * 根据 W_out/W_in 功率值和时间间隔计算累计电能，替换 PMBus 不可靠的 E_in/E_out 寄存器
   *
   * 公式: ΔE (Wh) = P (W) × Δt (s) / 3600
   * ESP32 每 2 秒推送一次数据，所以每次累积 ≈ P × 2 / 3600 Wh
   *
   * @param {Object} json 从 ESP32 收到的数据对象（会被修改）
   */
  _injectEnergyData(json) {
    // 只处理包含功率数据的推送（get_data 响应或自动推送）
    if (json.W_out === undefined && json.W_in === undefined) return
    if (json.ack !== undefined) return  // 命令确认响应，不处理

    const now = Date.now()
    const energy = this._energyData

    if (!energy.accumulated) {
      // 首次收到数据，只记录功率和时间，不累积
      // 不从 PMBus 读取 E_in/E_out 作为初始值，完全从 0 开始自行积分
      energy.lastW_out = json.W_out || 0
      energy.lastW_in = json.W_in || 0
      energy.lastTime = now
      energy.accumulated = true
    } else {
      // 计算时间间隔 (秒)
      const dt = (now - energy.lastTime) / 1000
      if (dt > 0 && dt < 60) {  // 防止长时间断开后累积异常值
        // 使用梯形积分: ΔE = (P_prev + P_curr) / 2 × Δt / 3600
        const avgW_out = (energy.lastW_out + (json.W_out || 0)) / 2
        const avgW_in = (energy.lastW_in + (json.W_in || 0)) / 2
        const dE_out = avgW_out * dt / 3600  // W × s / 3600 = Wh
        const dE_in = avgW_in * dt / 3600

        energy.E_out += dE_out
        energy.E_in += dE_in
      }
      // 更新上次值
      energy.lastW_out = json.W_out || 0
      energy.lastW_in = json.W_in || 0
      energy.lastTime = now
    }

    // 注入计算后的累计电能，覆盖 PMBus 的 E_in/E_out
    json.E_out = Math.round(energy.E_out * 10) / 10  // 保留 1 位小数
    json.E_in = Math.round(energy.E_in * 10) / 10
  }

  /**
   * 断开连接时重置累计电能（重新连接后重新开始累积）
   */
  _resetEnergy() {
    this._energyData = {
      E_out: 0,
      E_in: 0,
      lastW_out: 0,
      lastW_in: 0,
      lastTime: 0,
      accumulated: false
    }
  }

  /**
   * 发送数据 (JSON 格式)
   * @param {Object} data 要发送的数据对象
   */
  /**
   * 发送数据 (JSON 格式)
   * 自动分包: 每包不超过 20 字节 (MTU=23 时有效载荷)
   * @param {Object} data 要发送的数据对象
   */
  send(data) {
    return new Promise((resolve, reject) => {
      if (!this._deviceId || !this._serviceId || !this._rxCharId) {
        reject(new Error('未连接'))
        return
      }

      const jsonStr = JSON.stringify(data) + '\n'
      const totalBytes = this._stringToArrayBuffer(jsonStr).byteLength
      const CHUNK_SIZE = 18  // 每包 18 字节, 留 2 字节余量

      console.log('[BLE] 发送数据:', jsonStr.trim(), `(${totalBytes} bytes)`)

      if (totalBytes <= CHUNK_SIZE) {
        // 小数据, 直接发送
        const buffer = this._stringToArrayBuffer(jsonStr)
        uni.writeBLECharacteristicValue({
          deviceId: this._deviceId,
          serviceId: this._serviceId,
          characteristicId: this._rxCharId,
          value: buffer,
          writeType: 'write',
          success: () => {
            console.log('[BLE] 发送成功:', jsonStr.trim())
            resolve()
          },
          fail: (err) => {
            this._handleSendError(err, reject)
          }
        })
      } else {
        // 大数据, 分包发送
        console.log('[BLE] 分包发送:', totalBytes, 'bytes →', Math.ceil(totalBytes / CHUNK_SIZE), 'chunks')
        this._sendChunks(jsonStr, 0, CHUNK_SIZE, resolve, reject)
      }
    })
  }

  /**
   * 递归发送数据分片
   */
  _sendChunks(data, offset, chunkSize, resolve, reject) {
    const chunk = data.substring(offset, offset + chunkSize)
    const buffer = this._stringToArrayBuffer(chunk)
    const isLast = (offset + chunkSize >= data.length)

    uni.writeBLECharacteristicValue({
      deviceId: this._deviceId,
      serviceId: this._serviceId,
      characteristicId: this._rxCharId,
      value: buffer,
      writeType: 'write',
      success: () => {
        if (isLast) {
          console.log('[BLE] 分包发送完成 (共', Math.ceil(data.length / chunkSize), '包)')
          resolve()
        } else {
          // 等待 30ms 再发下一包, 给 BLE 栈处理时间
          setTimeout(() => {
            this._sendChunks(data, offset + chunkSize, chunkSize, resolve, reject)
          }, 30)
        }
      },
      fail: (err) => {
        this._handleSendError(err, reject)
      }
    })
  }

  /**
   * 处理发送错误
   */
  _handleSendError(err, reject) {
    console.error('[BLE] 发送失败:', err)
    // 如果错误是 no connection，自动重置连接状态
    if (err.errMsg && err.errMsg.indexOf('no connection') >= 0) {
      this._connected = false
      this._deviceId = ''
      this._serviceId = ''
      this._txCharId = ''
      this._rxCharId = ''
      this._receiveBuffer = ''
      this._notifyStatus('连接已断开')
    }
    reject(err)
  }

  /**
   * 将字符串转为 ArrayBuffer (兼容 uni-app Android)
   */
  _stringToArrayBuffer(str) {
    const len = str.length
    const buf = new ArrayBuffer(len)
    const view = new Uint8Array(buf)
    for (let i = 0; i < len; i++) {
      view[i] = str.charCodeAt(i) & 0xFF
    }
    return buf
  }

  /**
   * 将 ArrayBuffer 转为字符串 (兼容 uni-app Android, 替代 TextDecoder)
   */
  _arrayBufferToString(buf) {
    const view = new Uint8Array(buf)
    let result = ''
    for (let i = 0; i < view.length; i++) {
      result += String.fromCharCode(view[i])
    }
    return result
  }

  /**
   * 获取电源实时数据
   */
  getData() {
    return this.send({ cmd: 'get_data' })
  }

  /**
   * 设定输出电压
   * @param {number} voltage 电压值 (0-12V)
   */
  setVoltage(voltage) {
    return this.send({ cmd: 'set', V_set: parseFloat(voltage.toFixed(3)) })
  }

  /**
   * 设定输出电流
   * @param {number} current 电流值 (0-60A)
   */
  setCurrent(current) {
    return this.send({ cmd: 'set', I_set: parseFloat(current.toFixed(3)) })
  }

  /**
   * 电源开关
   * @param {boolean} on true=开机, false=关机
   */
  setPower(on) {
    return this.send({ cmd: 'set', power: on ? 1 : 0 })
  }

  /**
   * 获取设备信息
   */
  getInfo() {
    return this.send({ cmd: 'get_info' })
  }

  /**
   * 清除故障
   */
  clearFaults() {
    return this.send({ cmd: 'clear_faults' })
  }

  /**
   * 设置数据接收回调
   * 注册后立即用缓存数据回掉一次，避免新页面需等待下次推送才有数据显示
   * @param {Function} callback 回调函数 (data)
   */
  onData(callback) {
    this._onDataCallback = callback
    // 如果有缓存数据，立即回掉一次，让新页面秒显示
    if (this._lastData) {
      callback(this._lastData)
    }
  }

  /**
   * 设置状态回调
   * @param {Function} callback 回调函数 (status)
   */
  onStatus(callback) {
    this._onStatusCallback = callback
  }

  /**
   * 通知状态变化
   */
  _notifyStatus(status) {
    this._onStatusCallback && this._onStatusCallback(status)
  }

  /**
   * 断开连接
   */
  disconnect() {
    // #ifdef APP-PLUS
    if (this._deviceId) {
      uni.closeBLEConnection({
        deviceId: this._deviceId,
        success: () => {
          console.log('[BLE] 已断开连接')
        }
      })
    }
    // #endif
    this._connected = false
    this._deviceId = ''
    this._serviceId = ''
    this._txCharId = ''
    this._rxCharId = ''
    this._receiveBuffer = ''
    this._notifyStatus('已断开')
  }

  /**
   * 获取已连接设备的 deviceId
   */
  getDeviceId() {
    return this._deviceId
  }

  /**
   * 连接成功后自动获取设备信息（只请求一次，缓存后不再重复）
   */
  _requestDeviceInfo() {
    // 如果已经有缓存，不再重复请求
    if (this._deviceInfo) return
    // 延迟 500ms 发送，等连接稳定后再请求
    setTimeout(() => {
      this.send({ cmd: 'get_info' }).catch(err => {
        console.warn('[BLE] 自动获取设备信息失败:', err)
      })
    }, 500)
  }

  /**
   * 获取缓存的设备信息
   * 设备信息是固定不变的（MFR_ID/MFR_MODEL 等），连接后只获取一次
   * @returns {Object|null} 设备信息对象，未获取到时返回 null
   */
  getDeviceInfo() {
    return this._deviceInfo
  }

  /**
   * 是否已连接
   */
  get connected() {
    return this._connected
  }
}

// 单例导出
const bleService = new BleService()
export default bleService
