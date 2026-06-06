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
 *   请求: {"cmd":"calibrate","V_mult":1.005,"V_offset":-0.05}  → 电压校准
 *   请求: {"cmd":"cal_mode","enter":1} → 进入/退出校准模式
 *   请求: {"cmd":"get_info"}           → 获取设备信息
 */

// BLE 服务 UUID (与 ESP32-C3 端约定)
const BLE_SERVICE_UUID = '6E400001-B5A3-F393-E0A9-E50E24DCCA9E'
const BLE_TX_UUID = '6E400002-B5A3-F393-E0A9-E50E24DCCA9E' // ESP32 → App (通知)
const BLE_RX_UUID = '6E400003-B5A3-F393-E0A9-E50E24DCCA9E' // App → ESP32 (写入)

// 本地存储 key：保存上次连接的设备信息，用于自动重连
const STORAGE_KEY_LAST_DEVICE = 'ble_last_device'

class BleService {
  constructor() {
    this._deviceId = ''
    this._serviceId = ''
    this._txCharId = ''
    this._rxCharId = ''
    this._connected = false
    this._onDataCallback = null
    this._onStatusCallback = null
    this._onDeviceListCallback = null  // 设备列表变化回调
    this._receiveBuffer = ''
    this._initialized = false  // 防止重复初始化
    this._onBleOffCallback = null  // 蓝牙未开启回调
    this._connectionStateChangeRegistered = false  // 防止重复注册连接状态监听
    this._lastData = null  // 缓存最后一条数据，用于新页面注册时立即回掉
    this._currentConnectionSessionId = 0  // 当前连接 Session ID，断线重连时更新，旧监听器自动失效
    this._deviceInfo = null  // 缓存设备信息（连接成功后自动获取一次，固定不变）
    this._devices = []  // 扫描到的设备列表 [{deviceId, name, RSSI}]
    this._scanning = false  // 是否正在扫描

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
   *
   * 流程：
   * 1. Android 12+ → 先请求运行时权限（BLUETOOTH_SCAN / BLUETOOTH_CONNECT / 定位）
   * 2. 权限就绪 → 调用 uni.openBluetoothAdapter 打开蓝牙适配器
   * 3. 蓝牙未开启（errCode 10001）→ 触发系统级蓝牙开启请求
   * 4. 降级方案 → uni.showModal 引导用户手动开启
   *
   * @returns {Promise<boolean>} 初始化成功返回 true，失败（蓝牙未开启等）返回 false
   */
  init() {
    if (this._initialized) {
      console.log('[BLE] 蓝牙适配器已初始化，跳过')
      return Promise.resolve(true)
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
          this._deviceInfo = null  // 断开连接时清除设备信息，页面显示"离线"
          // 更新 Session ID，使旧监听器自动失效（替代不可靠的 off 方法）
          this._currentConnectionSessionId = Date.now()
          // 断开连接时重置累计电能（重新连接后重新开始累积）
          this._resetEnergy()
          this._notifyStatus('已断开')
          // 断开时触发 onData 回调通知所有页面数据已清空
          if (this._onDataCallback) {
            this._onDataCallback({_disconnected: true})
          }
          // 底层原生断线重连：延迟 3 秒后自动尝试重连上次设备
          setTimeout(() => {
            console.log('[BLE] 断线自动重连：3秒延迟到期，尝试重连')
            this.autoReconnect()
          }, 3000)
        } else {
          this._notifyStatus('已连接')
        }
      })
    }

    // 获取平台信息
    const sysInfo = uni.getSystemInfoSync()
    const platform = sysInfo.platform.toLowerCase() // 'android' | 'ios' | 'devtools'

    // ===== 第一步：请求蓝牙相关运行时权限（Android 12+ 必需） =====
    return new Promise((resolve) => {
      this._requestBlePermissions(platform, () => {
        // ===== 第二步：权限就绪，打开蓝牙适配器 =====
        this._openAdapter(platform, resolve)
      })
    })
  }

  /**
   * 请求蓝牙相关运行时权限（Android 12+ 必需）
   * @param {string} platform 'android' | 'ios' | 'devtools'
   * @param {Function} onReady 权限就绪回调
   */
  _requestBlePermissions(platform, onReady) {
    // iOS 和 开发者工具 不需要运行时权限请求
    if (platform !== 'android') {
      onReady()
      return
    }

    // ===== Android 12+ (API 31+) 动态权限申请 =====
    // 使用 plus.android.requestPermissions (Native.js) 替代 uni.requestSystemPermission，
    // 因为 Native.js 方式更底层、兼容性更好，能确保 BLUETOOTH_CONNECT 权限就绪，
    // 避免后续 Native.js 调用 BluetoothAdapter.ACTION_REQUEST_ENABLE 时抛出 SecurityException。
    try {
      if (typeof plus.android.requestPermissions === 'function') {
        const permissions = [
          'android.permission.BLUETOOTH_SCAN',
          'android.permission.BLUETOOTH_CONNECT',
          'android.permission.ACCESS_FINE_LOCATION'
        ]

        plus.android.requestPermissions(
          permissions,
          (result) => {
            // result: { granted: number[], denied: number[], deniedAlways: number[] }
            console.log('[BLE] Native.js 权限请求结果:', JSON.stringify(result))

            const grantedList = result.granted || []
            const deniedList = result.denied || []
            const deniedAlwaysList = result.deniedAlways || []

            // 检查 BLUETOOTH_CONNECT 是否被授予（这是 Native.js 唤醒蓝牙必需权限）
            const hasBluetoothConnect = grantedList.some(
              p => p === 'android.permission.BLUETOOTH_CONNECT' || p === 33  // 33 = BLUETOOTH_CONNECT 的常量值
            )

            if (hasBluetoothConnect) {
              console.log('[BLE] BLUETOOTH_CONNECT 权限已授予，继续初始化')
              onReady()
            } else if (deniedAlwaysList.length > 0) {
              // 用户勾选了"不再询问"，必须引导去设置页
              console.warn('[BLE] 权限被永久拒绝:', JSON.stringify(deniedAlwaysList))
              this._notifyStatus('需要蓝牙权限')
              this._showPermissionGuide()
            } else {
              // 权限被拒绝但未勾选"不再询问"，可以再次尝试
              console.warn('[BLE] 权限被拒绝（可重试）:', JSON.stringify(deniedList))
              // 给用户一个提示，然后继续尝试（系统可能还会弹窗）
              uni.showToast({ title: '请授予蓝牙权限', icon: 'none' })
              onReady()
            }
          },
          (error) => {
            // plus.android.requestPermissions 本身调用失败
            console.error('[BLE] plus.android.requestPermissions 调用异常:', JSON.stringify(error))
            // 降级：尝试继续，系统可能在 openBluetoothAdapter 时弹窗
            onReady()
          }
        )
      } else {
        // 低版本 HBuilderX 不支持 plus.android.requestPermissions
        console.log('[BLE] 当前环境不支持 plus.android.requestPermissions，跳过运行时权限请求')
        onReady()
      }
    } catch (e) {
      console.error('[BLE] Native.js 权限请求异常，降级继续:', e)
      onReady()
    }
  }

  /**
   * 打开蓝牙适配器（核心方法）
   * @param {string} platform 'android' | 'ios' | 'devtools'
   */
  _openAdapter(platform, onComplete) {
    uni.openBluetoothAdapter({
      success: () => {
        console.log('[BLE] 蓝牙适配器初始化成功')
        this._notifyStatus('蓝牙就绪')
        // 通知 init() 的 Promise：初始化成功，允许后续 autoReconnect
        if (onComplete) onComplete(true)
      },
      fail: (err) => {
        console.log('[BLE] 蓝牙初始化彻底失败:', err)
        const errCode = err.errCode !== undefined ? err.errCode : err.code
        const errMsg = (err.errMsg || '').toLowerCase()

        // ===== 错误码分类处理 =====
        // 注意: 部分 Android 系统（如小米）errCode 可能为 undefined，需通过 errMsg 字符串匹配
        const isBleOff = errCode === 10001 ||
                         (errMsg.indexOf('10001') >= 0) ||
                         (errMsg.indexOf('not turn on') >= 0) ||
                         (errMsg.indexOf('bluetooth is not on') >= 0) ||
                         (errMsg.indexOf('bluetooth not open') >= 0) ||
                         (errMsg.indexOf('not available') >= 0)

        const isPermissionDenied = errCode === 10013 ||
                                   (errMsg.indexOf('10013') >= 0) ||
                                   (errMsg.indexOf('permission') >= 0) ||
                                   (errMsg.indexOf('denied') >= 0) ||
                                   (errMsg.indexOf('reject') >= 0)

        if (isBleOff) {
          // 错误码 10001: 蓝牙未开启（Android/iOS 通用）
          console.log('[BLE] 检测到蓝牙未开启，准备调用 Native.js 唤醒...')

          // 解决 this 指向可能丢失的问题
          const platform = uni.getSystemInfoSync().platform.toLowerCase()
          if (typeof this._handleBleOff === 'function') {
            this._handleBleOff(platform)
          } else {
            console.error('[BLE] 找不到 _handleBleOff 方法，唤醒失败！')
          }

          // 关键阻断：蓝牙未开启时，通知 init() 返回 false，截断后续 autoReconnect
          if (onComplete) onComplete(false)
        } else if (isPermissionDenied) {
          // 错误码 10013: 未授予必要权限（Android）
          this._notifyStatus('缺少蓝牙权限')
          this._showPermissionGuide()
          if (onComplete) onComplete(false)
        } else {
          // 其他未知错误 - 弹窗显示完整错误信息用于诊断
          console.error('[BLE] 蓝牙初始化彻底失败:', err)
          uni.showModal({
            title: '蓝牙异常拦截',
            content: '底层报错信息: ' + JSON.stringify(err),
            showCancel: false
          })
          this._notifyStatus('蓝牙不可用')
          if (onComplete) onComplete(false)
        }
      }
    })
  }

  /**
   * 处理蓝牙未开启的情况
   * 尝试触发系统级蓝牙开启弹窗
   * @param {string} platform 'android' | 'ios' | 'devtools'
   */
  _handleBleOff(platform) {
    if (platform === 'android') {
      // === Android 策略：使用 Native.js 触发系统原生蓝牙开启弹窗 ===
      // 通过 plus.android 调用 Android 原生 API：
      //   BluetoothAdapter.ACTION_REQUEST_ENABLE 会弹出系统级底部对话框
      //   用户点击"允许"后系统自动开启蓝牙，无需跳转设置页
      //
      // 注意：调用此方法前，_requestBlePermissions 必须已成功申请 BLUETOOTH_CONNECT 权限，
      // 否则 startActivity 会抛出 SecurityException。
      //
      // 坑点记录：不要用 plus.android.importClass('android.bluetooth.BluetoothAdapter')
      // 去反射获取 ACTION_REQUEST_ENABLE 静态常量，某些 ROM 会返回 undefined 导致崩溃。
      // 直接写死底层魔法字符串 'android.bluetooth.adapter.action.REQUEST_ENABLE' 最稳。
      try {
        const main = plus.android.runtimeMainActivity()
        const Intent = plus.android.importClass('android.content.Intent')
        // 直接传入安卓底层原生字符串，避开类属性反射坑
        const intent = new Intent('android.bluetooth.adapter.action.REQUEST_ENABLE')
        main.startActivity(intent)

        console.log('[BLE] 已触发系统原生蓝牙开启弹窗')

        // 延迟检测蓝牙是否已开启（给用户操作时间）
        setTimeout(() => {
          uni.openBluetoothAdapter({
            success: () => {
              console.log('[BLE] 用户已通过系统弹窗开启蓝牙')
              this._notifyStatus('蓝牙就绪')
              uni.showToast({ title: '蓝牙已开启', icon: 'success' })
              this.autoReconnect()
            },
            fail: () => {
              console.warn('[BLE] 用户未通过系统弹窗开启蓝牙')
              this._showBleSettingsGuide()
            }
          })
        }, 3000)
      } catch (e) {
        // Native.js 调用失败时的降级方案
        // 可能原因：BLUETOOTH_CONNECT 权限未授予、设备不支持等
        console.error("【极其重要】Native.js 唤醒蓝牙严重报错:", e.message || e)
        console.error('[BLE] Native.js 调用失败，降级为 showModal 方案:', e)
        uni.showModal({
          title: '需要开启蓝牙',
          content: '本应用需要通过蓝牙连接电源设备，请允许开启蓝牙。',
          confirmText: '开启蓝牙',
          cancelText: '暂不开启',
          success: (res) => {
            if (res.confirm) {
              uni.openBluetoothAdapter({
                success: () => {
                  console.log('[BLE] 用户已开启蓝牙')
                  this._notifyStatus('蓝牙就绪')
                  uni.showToast({ title: '蓝牙已开启', icon: 'success' })
                  this.autoReconnect()
                },
                fail: (retryErr) => {
                  console.warn('[BLE] 用户仍未开启蓝牙:', JSON.stringify(retryErr))
                  this._showBleSettingsGuide()
                }
              })
            }
          }
        })
      }
    } else if (platform === 'ios') {
      // === iOS 策略 ===
      // iOS 上无法通过 Native.js 触发系统蓝牙开启弹窗，使用 showModal 引导
      uni.showModal({
        title: '蓝牙未开启',
        content: '检测到蓝牙未开启，请前往系统设置中打开蓝牙',
        showCancel: false,
        confirmText: '我知道了'
      })
    } else {
      // 开发者工具
      this._notifyStatus('蓝牙未开启（请使用真机调试）')
    }
  }

  /**
   * 引导用户去系统设置页开启蓝牙
   */
  _showBleSettingsGuide() {
    uni.showModal({
      title: '无法开启蓝牙',
      content: '无法自动开启蓝牙，请前往系统设置中手动开启蓝牙后重试。',
      confirmText: '去设置',
      cancelText: '稍后',
      success: (res) => {
        if (res.confirm) {
          uni.openAppSetting({
            success: () => {
              console.log('[BLE] 已跳转到系统设置页')
            }
          })
        }
      }
    })
  }

  /**
   * 引导用户去系统设置页授予权限
   */
  _showPermissionGuide() {
    uni.showModal({
      title: '需要蓝牙权限',
      content: '本应用需要蓝牙权限才能连接电源设备，请在设置中授予「附近设备」或「位置信息」权限。',
      confirmText: '去设置',
      cancelText: '稍后',
      success: (res) => {
        if (res.confirm) {
          uni.openAppSetting({
            success: () => {
              console.log('[BLE] 已跳转到系统设置页')
            }
          })
        }
      }
    })
  }

  /**
   * 开始扫描 BLE 设备
   * @param {Function} onFound 发现设备回调 (device) - 可选，兼容旧用法
   */
  startScan(onFound) {
    // #ifdef APP-PLUS
    if (this._scanning) {
      console.log('[BLE] 已在扫描中，跳过')
      return
    }
    this._scanning = true
    this._devices = []  // 清空设备列表
    this._notifyDeviceList()  // 通知空列表

    uni.startBluetoothDevicesDiscovery({
      allowDuplicatesKey: false,  // 不重复上报，减少重复
      success: () => {
        console.log('[BLE] 开始扫描设备')
        this._notifyStatus('正在扫描...')
      },
      fail: (err) => {
        console.error('[BLE] 扫描失败:', err)
        this._scanning = false
        this._notifyStatus('扫描失败')
      }
    })

    // 监听发现设备
    uni.onBluetoothDeviceFound((res) => {
      const devices = res.devices
      let changed = false
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

        if (!hasNusService) continue

        // 去重：检查设备是否已在列表中
        const existing = this._devices.find(d => d.deviceId === device.deviceId)
        if (existing) {
          // 更新 RSSI
          existing.RSSI = device.RSSI
        } else {
          // 添加新设备
          this._devices.push({
            deviceId: device.deviceId,
            name: device.name || '未知设备',
            RSSI: device.RSSI
          })
          changed = true
        }
      }
      if (changed) {
        this._notifyDeviceList()
      }
      // 兼容旧用法：onFound 回调
      onFound && onFound(device)
    })
    // #endif
  }

  /**
   * 获取当前扫描到的设备列表
   */
  getDeviceList() {
    return [...this._devices]
  }

  /**
   * 注册设备列表变化回调
   * @param {Function} callback 回调函数 (devices)
   */
  onDeviceList(callback) {
    this._onDeviceListCallback = callback
    // 立即回调当前列表
    if (this._devices.length > 0) {
      callback([...this._devices])
    }
  }

  /**
   * 通知设备列表变化
   */
  _notifyDeviceList() {
    this._onDeviceListCallback && this._onDeviceListCallback([...this._devices])
  }

  /**
   * 是否正在扫描
   */
  get scanning() {
    return this._scanning
  }

  /**
   * 停止扫描
   */
  stopScan() {
    // #ifdef APP-PLUS
    this._scanning = false
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
   * @param {string} [deviceName] 设备名称（可选，用于保存到本地存储）
   */
  connect(deviceId, deviceName) {
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
              this._discoverServices().then(() => {
                // 连接完全成功后，保存设备信息到本地存储（下次启动自动重连）
                // 策略：始终使用传入的 deviceName（扫描到的广播名），
                // 如果无效则清空名称，让显示逻辑回退到 deviceId 后缀
                let nameToSave = ''
                if (deviceName && !this._isNameGarbled(deviceName)) {
                  nameToSave = deviceName
                  console.log('[BLE] 保存广播名:', nameToSave)
                } else {
                  console.log('[BLE] 广播名无效，清空名称')
                }
                this._saveLastDevice(deviceId, nameToSave)
                resolve()
              }).catch((err) => {
                // 发现服务失败，重置连接状态
                console.error('[BLE] 发现服务失败:', err)
                this._connected = false
                this._deviceId = ''
                this._notifyStatus('连接失败')
                reject(err)
              })
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
   * 自动重连上次连接的设备
   * 在 App 启动时调用，如果本地存储中有上次连接的设备信息，自动尝试连接
   * @returns {Promise<boolean>} 是否成功发起重连（不代表连接成功）
   */
  async autoReconnect(retryCount = 0) {
    const lastDevice = this._getLastDevice()
    if (!lastDevice) {
      console.log('[BLE] 没有上次连接的设备，跳过自动重连')
      return false
    }
    if (this._connected) {
      console.log('[BLE] 已连接，跳过自动重连')
      return false
    }
    console.log('[BLE] 尝试自动重连 (第' + (retryCount + 1) + '次):', lastDevice.name || lastDevice.deviceId)
    this._notifyStatus('自动重连中...')
    try {
      // 自动重连时不传旧名字，避免覆盖 ESP32 当前实际的广播名
      // 传空字符串让 connect() 保留本地已有名称不变
      await this.connect(lastDevice.deviceId, '')
      console.log('[BLE] 自动重连成功:', lastDevice.name || lastDevice.deviceId)
      this._notifyStatus('已连接')
      return true
    } catch (err) {
      console.warn('[BLE] 自动重连失败:', err.message || err)
      // 重试机制：最多重试 3 次，间隔递增（3s, 6s, 12s）
      if (retryCount < 3) {
        const delay = (retryCount + 1) * 3000
        console.log('[BLE] 将在 ' + (delay / 1000) + ' 秒后重试...')
        this._notifyStatus('重连失败，' + (delay / 1000) + '秒后重试...')
        setTimeout(() => {
          this.autoReconnect(retryCount + 1)
        }, delay)
        return false
      }
      this._notifyStatus('自动重连失败，请手动连接')
      return false
    }
  }

  /**
   * 获取可读的设备显示名称
   * 如果 name 为空或乱码，使用 deviceId 的后 8 位作为显示名
   */
  _getDisplayName(deviceId, name) {
    if (name && name.trim().length > 0) return name.trim()
    // name 不可用时，用 deviceId 后 8 位
    if (deviceId && deviceId.length > 8) {
      return '设备(' + deviceId.substring(deviceId.length - 8).toUpperCase() + ')'
    }
    return '已连接设备'
  }

  /**
   * 检测名称是否为乱码（包含太多控制字符或非UTF-8序列）
   * 蓝牙扫描返回的名称有时会包含乱码，需要过滤
   */
  _isNameGarbled(name) {
    if (!name || name.trim().length === 0) return true
    let controlCount = 0
    for (let i = 0; i < name.length; i++) {
      const code = name.charCodeAt(i)
      // 控制字符 (0x00-0x1F, 0x7F) 且不是常见的换行/回车/制表符
      if ((code > 0 && code < 0x08) || (code > 0x0D && code < 0x20) || code === 0x7F) {
        controlCount++
      }
      // 代理项（未配对的UTF-16代理）也是乱码
      if (code >= 0xD800 && code <= 0xDFFF) {
        controlCount++
      }
    }
    // 如果超过 30% 的字符是控制字符，认为是乱码
    return (controlCount / name.length) > 0.3
  }

  /**
   * 保存上次连接的设备信息到本地存储
   * 如果新名称为乱码且本地已有有效名称，则不覆盖
   */
  _saveLastDevice(deviceId, name) {
    try {
      const cleanName = (name || '').trim()
      
      // 如果新名称为空，直接保存空名称（覆盖旧的），让显示逻辑回退到 deviceId
      if (!cleanName) {
        const info = { deviceId, name: '', time: Date.now() }
        uni.setStorageSync(STORAGE_KEY_LAST_DEVICE, info)
        console.log('[BLE] 已清空设备名称')
        return
      }
      
      // 乱码检测：如果新名称是乱码，且本地已有有效名称，则不覆盖
      if (this._isNameGarbled(cleanName)) {
        try {
          const existing = uni.getStorageSync(STORAGE_KEY_LAST_DEVICE)
          if (existing && existing.deviceId === deviceId &&
              existing.name && existing.name.trim().length > 0 &&
              !this._isNameGarbled(existing.name)) {
            console.log('[BLE] 扫描名称为乱码，保留本地有效名称:', existing.name)
            return
          }
        } catch (e) {}
      }
      
      const info = { deviceId, name: cleanName, time: Date.now() }
      uni.setStorageSync(STORAGE_KEY_LAST_DEVICE, info)
      console.log('[BLE] 已保存设备信息到本地:', this._getDisplayName(deviceId, cleanName))
    } catch (err) {
      console.warn('[BLE] 保存设备信息失败:', err)
    }
  }

  /**
   * 从本地存储读取上次连接的设备信息
   * @returns {Object|null} { deviceId, name, time } 或 null
   */
  _getLastDevice() {
    try {
      const info = uni.getStorageSync(STORAGE_KEY_LAST_DEVICE)
      if (info && info.deviceId) {
        return info
      }
    } catch (err) {
      console.warn('[BLE] 读取设备信息失败:', err)
    }
    return null
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
   * 开始监听数据（闭包 Session 拦截法）
   *
   * 核心思路:
   *   uni-app 的 uni.offBLECharacteristicValueChange() 在 Android 上完全无效,
   *   每次重连后 onBLECharacteristicValueChange 会叠加新的监听器。
   *   解决方案: 每次连接生成唯一 Session ID, 回调中检查 Session 是否匹配,
   *   不匹配则直接丢弃数据, 实现"旧监听器自动失效"。
   *
   *   同时, 每次注册时重置 _receiveBuffer, 并丢弃注册后短时间内收到的数据
   *   （用于排空底层 BLE 协议栈残留的旧连接脏数据）。
   */
  _startListening() {
    // 生成新的连接 Session ID，使旧监听器自动失效
    this._currentConnectionSessionId = Date.now()
    const mySessionId = this._currentConnectionSessionId

    // 重置接收缓冲区
    this._receiveBuffer = ''

    // 注册防抖: 丢弃注册后 800ms 内收到的数据（排空底层残留脏数据）
    const debounceEnd = Date.now() + 800

    console.log('[BLE] 注册特征值变化监听器, Session:', mySessionId)

    // 注册监听器（每次重连都注册新的，旧的靠 Session 机制自动失效）
    uni.onBLECharacteristicValueChange((res) => {
      // === Session 检查：如果已经不是最新的监听器，直接丢弃（静默） ===
      if (mySessionId !== this._currentConnectionSessionId) {
        return
      }

      // 直接将 ArrayBuffer 转为字符串
      const text = this._arrayBufferToString(res.value)

      // === 防抖窗口：注册后 800ms 内只排空不解析 ===
      if (Date.now() < debounceEnd) {
        this._receiveBuffer += text
        const lines = this._receiveBuffer.split('\n')
        this._receiveBuffer = lines.pop() || ''
        if (this._receiveBuffer.length > 2048) {
          this._receiveBuffer = ''
        }
        return
      }

      // === 正常接收 ===
      this._receiveBuffer += text
      const lines = this._receiveBuffer.split('\n')
      this._receiveBuffer = lines.pop() || ''

      // 安全保护: 超过 2KB 无换行则清空
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
            if (json.MFR_ID) {
              this._deviceInfo = json
            }
            this._injectEnergyData(json)
            this._lastData = json
            this._onDataCallback && this._onDataCallback(json)
          } catch (e) {
            // 静默处理，防止控制台被半截子脏数据刷屏
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
   * @param {number} current 电流值 (0-62.5A)
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
   * 进入电压校准模式
   */
  enterCalMode() {
    return this.send({ cmd: 'cal_mode', enter: 1 })
  }

  /**
   * 退出电压校准模式
   */
  exitCalMode() {
    return this.send({ cmd: 'cal_mode', enter: 0 })
  }

  /**
   * 电压校准
   * @param {number} multiplier 乘数修正 (默认 1.0)
   * @param {number} offset 偏置修正 (默认 0.0V)
   */
  /**
   * 发送电压校准参数到设备
   * @param {Object} params - 校准参数
   * @param {number} [params.V_mult] - 电压乘数修正
   * @param {number} [params.V_offset] - 电压偏置修正
   */
  calibrate(params = {}) {
    const payload = { cmd: 'calibrate' }
    if (params.V_mult !== undefined) {
      payload.V_mult = parseFloat(params.V_mult.toFixed(4))
    }
    if (params.V_offset !== undefined) {
      payload.V_offset = parseFloat(params.V_offset.toFixed(4))
    }
    return this.send(payload)
  }

  /**
   * 校准模式: PWM 增加 (UP 按键)
   */
  calPwmUp() {
    return this.send({ cmd: 'cal_pwm_adjust', dir: 1 })
  }

  /**
   * 校准模式: PWM 减少 (DOWN 按键)
   */
  calPwmDown() {
    return this.send({ cmd: 'cal_pwm_adjust', dir: -1 })
  }

  /**
   * 校准模式: 确认当前校准点 (OK 按键)
   */
  calConfirm() {
    return this.send({ cmd: 'cal_confirm' })
  }

  /**
   * 设置电流校准表 (6 点分段线性插值)
   * @param {Array} points - 6 个校准点数组, 每个点格式: {r: raw_val, v: target, p: pwm_val}
   *   r: PMBus 原始电流读数 (A)
   *   v: 目标电流 (A)
   *   p: PWM 值 (可选)
   */
  setICalTable(points) {
    const payload = {
      cmd: 'set_i_cal_table',
      points: points.map(p => ({
        r: parseFloat(p.r.toFixed(3)),
        v: parseFloat(p.v.toFixed(1)),
        p: p.p !== undefined ? parseFloat(p.p.toFixed(1)) : 0.0
      }))
    }
    return this.send(payload)
  }

  /**
   * 设置全局量程 (通用化校准前置步骤)
   * @param {number} vMax - 最大输出电压 (V)
   * @param {number} iMax - 最大输出电流 (A)
   */
  setRange(vMax, iMax) {
    return this.send({
      cmd: 'set_range',
      v_max: parseFloat(vMax.toFixed(1)),
      i_max: parseFloat(iMax.toFixed(1))
    })
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
   * 设置蓝牙未开启回调
   * 当蓝牙适配器初始化失败且检测到蓝牙未开启时触发
   * @param {Function} callback 回调函数
   */
  onBleOff(callback) {
    this._onBleOffCallback = callback
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
   * 获取最后一次收到的遥测数据（供设置页读取校准参数等）
   * @returns {Object|null}
   */
  getLastData() {
    return this._lastData
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
