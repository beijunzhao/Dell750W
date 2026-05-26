<template>
	<view class="container">

		<!-- 右上角蓝牙入口 -->
		<view class="header-bar">
			<view class="header-title">数控电源</view>
			<view class="header-actions">
				<view class="header-btn" @tap="showBleDeviceList">
					<text class="header-btn-icon">📡</text>
					<text class="header-btn-label">{{ connected ? '已连接' : '蓝牙' }}</text>
				</view>
				<view class="nav-ble" :class="{ connected: connected }">
					<view class="nav-ble-dot" :class="{ active: connected }"></view>
				</view>
			</view>
		</view>

		<!-- 蓝牙设备列表弹窗 -->
		<view class="ble-modal-overlay" v-if="showBleModal" @tap="closeBleModal">
			<view class="ble-modal-dialog" @tap.stop>
				<view class="ble-modal-header">
					<text class="ble-modal-title">{{ connected ? '已连接设备' : '选择蓝牙设备' }}</text>
					<text class="ble-modal-close" @tap="closeBleModal">✕</text>
				</view>

				<!-- 已连接时显示当前设备信息和断开按钮 -->
					<view v-if="connected" class="ble-connected-info">
						<view class="ble-connected-device">
							<text class="ble-connected-icon">📡</text>
							<view class="ble-connected-detail">
								<text class="ble-connected-name">{{ bleConnectedName || '已连接设备' }}</text>
								<text class="ble-connected-status">已连接</text>
							</view>
						</view>
						<view class="ble-btn ble-btn-disconnect" @tap="disconnectDevice">断开连接</view>
					</view>
	

				<!-- 扫描状态 -->
				<view v-if="!connected" class="ble-scan-status">
					<text v-if="scanning" class="ble-scanning-text">🔍 正在扫描设备...</text>
					<text v-else class="ble-scan-idle">点击下方按钮开始扫描</text>
				</view>

				<!-- 设备列表 -->
				<view v-if="!connected" class="ble-device-list">
					<view
						v-for="device in bleDevices"
						:key="device.deviceId"
						class="ble-device-item"
						@tap="connectDevice(device)"
					>
						<text class="ble-device-icon">📡</text>
						<view class="ble-device-info">
							<text class="ble-device-name">{{ device.name }}</text>
							<text class="ble-device-id">{{ device.deviceId }}</text>
						</view>
						<view class="ble-device-rssi">
							<text class="ble-rssi-bar" :class="rssiClass(device.RSSI)">{{ device.RSSI || 0 }}</text>
						</view>
					</view>
					<!-- 空状态 -->
					<view v-if="!scanning && bleDevices.length === 0" class="ble-empty">
						<text class="ble-empty-text">未发现设备</text>
					</view>
				</view>

				<!-- 操作按钮 -->
				<view v-if="!connected" class="ble-modal-footer">
					<view v-if="!scanning" class="ble-btn ble-btn-scan" @tap="startBleScan">开始扫描</view>
					<view v-else class="ble-btn ble-btn-stop" @tap="stopBleScan">停止扫描</view>
				</view>
			</view>
		</view>

		<!-- 电源状态 -->
		<view class="power-status-section">
			<!-- 效率标签 - 右上角 -->
			<view class="power-efficiency-badge" :class="{ active: powerOn && powerData.W_out > 0 }">
				<text class="power-efficiency-text">{{ efficiencyText }}</text>
			</view>
			<view class="power-toggle" :class="{ on: powerOn }" @tap="togglePower">
				<text class="power-icon">{{ powerOn ? '⏻' : '⏻' }}</text>
				<text class="power-label">{{ powerOn ? '运行中' : '已关闭' }}</text>
			</view>
			<view class="power-info">
				<text class="power-info-label">电源状态</text>
				<view class="power-info-row">
					<text class="power-info-value" :class="{ on: powerOn }">
						{{ powerOn ? '运行中' : '已关闭' }}
					</text>
				</view>
				<!-- 能量流光条 - 有输出时流动 -->
				<view class="energy-flow" :class="{ active: powerOn && powerData.W_out > 0 }">
					<view class="energy-flow-track">
						<view class="energy-flow-bar"></view>
					</view>
				</view>
			</view>
		</view>

		<!-- 电压/电流设置 - 并排显示 -->
		<view class="meter-row">
			<view class="meter-card voltage-card" @tap="showVoltageInput">
				<view class="meter-label">输出电压 <text class="tap-hint">点击设置</text></view>
				<view class="meter-value">
					<text class="value-number">{{ voltageSet.toFixed(2) }}</text>
					<text class="value-unit">V</text>
				</view>
				<view class="meter-bar">
					<view class="meter-bar-fill voltage-fill" :style="{ width: voltagePercent + '%' }"></view>
				</view>
			</view>

			<view class="meter-card current-card" @tap="showCurrentInput">
				<view class="meter-label">输出电流 <text class="tap-hint">点击设置</text></view>
				<view class="meter-value">
					<text class="value-number">{{ currentSet.toFixed(2) }}</text>
					<text class="value-unit">A</text>
				</view>
				<view class="meter-bar">
					<view class="meter-bar-fill current-fill" :style="{ width: currentPercent + '%' }"></view>
				</view>
			</view>
		</view>

		<!-- 输出功率 / 累计电能 -->
		<view class="info-grid">
			<view class="info-card power-card">
				<text class="info-label">输出功率</text>
				<text class="info-value">{{ powerData.W_out.toFixed(1) }} <text class="info-unit">W</text></text>
			</view>
			<view class="info-card power-card">
				<text class="info-label">累计输出电能</text>
				<text class="info-value">{{ powerData.E_out.toFixed(1) }} <text class="info-unit">Wh</text></text>
			</view>
		</view>

		<!-- 快速预设 -->
		<view class="preset-section">
			<view class="preset-header">
				<text class="preset-title">快速预设</text>
				<view class="preset-header-actions">
					<text class="preset-edit-btn" :class="{ editing: presetEditing }" @tap="togglePresetEdit">
						{{ presetEditing ? '✏️ 完成编辑' : '✏️ 编辑' }}
					</text>
				</view>
			</view>
			<view class="preset-grid">
				<view
					v-for="(preset, index) in presets"
					:key="preset.label"
					class="preset-btn"
					:class="{ 'preset-btn-editing': presetEditing }"
					@tap="presetEditing ? null : confirmApplyPreset(preset)"
				>
					<!-- 编辑模式：显示输入框 -->
					<template v-if="presetEditing">
						<input class="preset-edit-label-input" type="text" v-model="preset.label"
							placeholder="名称" @click.stop />
						<view class="preset-edit-inline">
							<input class="preset-edit-inline-input" type="digit"
								:value="preset.voltage" @input="e => onPresetEditInput(index, 'voltage', e)"
								placeholder="V" @click.stop />
							<text class="preset-edit-slash">/</text>
							<input class="preset-edit-inline-input" type="digit"
								:value="preset.current" @input="e => onPresetEditInput(index, 'current', e)"
								placeholder="A" @click.stop />
						</view>
					</template>
					<!-- 默认模式：显示预设值 -->
					<template v-else>
						<text class="preset-label">{{ preset.label }}</text>
						<text class="preset-detail">{{ preset.voltage }}V / {{ preset.current }}A</text>
					</template>
				</view>
			</view>
		</view>

		<!-- 状态 -->
		<view class="section-title">状态</view>
		<view class="status-grid">
			<view class="status-chip" :class="{ ok: powerOn }">
				{{ powerOn ? '运行中' : '已关闭' }}
			</view>
			<view class="status-chip" :class="{ ok: deviceOnline }">
				{{ deviceOnline ? '设备在线' : '设备离线' }}
			</view>
		<view class="status-chip ok" style="font-size:20rpx">
					{{ calText }}
				</view>
		</view>

		<!-- 自定义设置弹窗 -->
		<view class="modal-overlay" v-if="showModal" @tap="closeModal">
			<view class="modal-dialog" @tap.stop>
				<view class="modal-header">
					<text class="modal-title">{{ modalTitle }}</text>
					<text class="modal-close" @tap="closeModal">✕</text>
				</view>
				<view class="modal-body">
					<view class="modal-current">
						<text class="modal-current-label">当前值</text>
						<text class="modal-current-value" :class="modalType">{{ modalCurrentValue }}</text>
						<text class="modal-current-unit">{{ modalUnit }}</text>
					</view>
					<view class="modal-input-group">
						<text class="modal-input-label">目标值</text>
						<view class="modal-input-row">
							<input class="modal-input" type="digit" v-model="modalInputValue"
								:placeholder="`0-${modalMax}`" focus
								@confirm="confirmModal" />
							<text class="modal-input-unit">{{ modalUnit }}</text>
						</view>
					</view>
					<view class="modal-range">
						<text class="modal-range-text">范围: 0 ~ {{ modalMax }}{{ modalUnit }}</text>
					</view>
					<!-- 电压设置弹窗中增加校准入口 -->
					<view v-if="modalType === 'voltage'" class="modal-cal-section">
						<view class="modal-cal-divider"></view>
						<view class="modal-cal-btn" @tap="goCalibration">
							<text class="modal-cal-icon">🔧</text>
							<text class="modal-cal-text">电压校准 (6 点详细校准)</text>
							<text class="modal-cal-arrow">›</text>
						</view>
					</view>
				</view>
				<view class="modal-footer">
					<view class="modal-btn modal-btn-cancel" @tap="closeModal">取消</view>
					<view class="modal-btn modal-btn-confirm" :class="modalType" @tap="confirmModal">确认设置</view>
				</view>
			</view>
		</view>

		<!-- 预设确认应用弹窗 -->
		<view class="modal-overlay" v-if="showPresetConfirm" @tap="closePresetConfirm">
			<view class="modal-dialog preset-confirm-dialog" @tap.stop>
				<view class="modal-header">
					<text class="modal-title">应用预设</text>
					<text class="modal-close" @tap="closePresetConfirm">✕</text>
				</view>
				<view class="modal-body">
					<view class="preset-confirm-icon">⚡</view>
					<text class="preset-confirm-label">{{ presetConfirmData.label }}</text>
					<view class="preset-confirm-values">
						<view class="preset-confirm-item">
							<text class="preset-confirm-item-label">电压</text>
							<text class="preset-confirm-item-value voltage">{{ presetConfirmData.voltage }} <text class="preset-confirm-item-unit">V</text></text>
						</view>
						<view class="preset-confirm-divider"></view>
						<view class="preset-confirm-item">
							<text class="preset-confirm-item-label">电流</text>
							<text class="preset-confirm-item-value current">{{ presetConfirmData.current }} <text class="preset-confirm-item-unit">A</text></text>
						</view>
					</view>
				</view>
				<view class="modal-footer preset-confirm-footer">
					<view class="modal-btn modal-btn-cancel" @tap="closePresetConfirm">取消</view>
					<view class="modal-btn modal-btn-confirm voltage" @tap="doApplyPreset">确认应用</view>
				</view>
			</view>
		</view>

		<!-- 底部导航 -->
		<view class="bottom-nav">
			<view class="nav-item active" @tap="switchTab('index')">
				<text class="nav-icon">🏠</text>
				<text class="nav-label">首页</text>
			</view>
			<view class="nav-item" @tap="switchTab('control')">
				<text class="nav-icon">📊</text>
				<text class="nav-label">监控</text>
			</view>
			<view class="nav-item" @tap="switchTab('settings')">
				<text class="nav-icon">ℹ️</text>
				<text class="nav-label">信息</text>
			</view>
			<!-- 蓝牙状态 -->
			<view class="nav-ble" :class="{ connected: connected }">
				<view class="nav-ble-dot" :class="{ active: connected }"></view>
			</view>
		</view>
	</view>
</template>

<script>
	import bleService from '../../utils/ble-service.js'

	export default {
		data() {
			return {
				connected: false,
				deviceOnline: false,
				calMult: 1.0,
				calOffset: 0.0,
				statusText: '点击连接设备',
				powerOn: false,
				voltageSet: 0,
				currentSet: 0,
				voltageMax: 12.0,
				currentMax: 62.5,
				powerData: {
					W_out: 0, W_in: 0, E_out: 0
				},
				presets: [
					{ label: '低压小电流', voltage: 3.3, current: 1.0 },
					{ label: '5V 2A', voltage: 5.0, current: 2.0 },
					{ label: '12V 5A', voltage: 12.0, current: 5.0 },
					{ label: '12V 10A', voltage: 12.0, current: 10.0 },
					{ label: '12V 20A', voltage: 12.0, current: 20.0 },
					{ label: '满载', voltage: 12.0, current: 62.5 }
				],
				// 自定义弹窗状态
				showModal: false,
				modalType: 'voltage', // 'voltage' | 'current'
				modalInputValue: '',
				// 蓝牙设备列表弹窗
				showBleModal: false,
				bleDevices: [],
				scanning: false,
				bleConnectedName: '',
				// 预设功能
				presetEditing: false,       // 是否处于编辑模式
				showPresetConfirm: false,   // 确认应用弹窗
				presetConfirmData: { label: '', voltage: 0, current: 0 }
			}
		},
		onLoad() {
			bleService.onStatus((status) => {
				this.statusText = status
				this.connected = bleService.connected
				if (this.connected) {
					this._updateBleConnectedName()
				}
			})

			bleService.onData((data) => {
				if (data.power_on !== undefined) {
					this.powerOn = data.power_on === 1
				}
					if (data.device_online !== undefined) {
						this.deviceOnline = data.device_online === true || data.device_online === "true"
					}
					if (data.V_mult !== undefined) {
						this.calMult = data.V_mult
					}
					if (data.V_offset !== undefined) {
						this.calOffset = data.V_offset
				}
				if (data.V_out !== undefined) {
					this.voltageSet = data.V_out
				}
				if (data.I_out !== undefined) {
					this.currentSet = data.I_out
				}
				// 更新功率和电能数据
				if (data.W_out !== undefined || data.W_in !== undefined || data.E_out !== undefined) {
					this.powerData = { ...this.powerData, ...data }
				}
				// 处理设置命令的确认响应，同步电源状态
				if (data.ack === 'ok' && data.power !== undefined) {
					this.powerOn = data.power === 1
				}
				// 处理设置命令的确认响应
				if (data.ack === 'ok' && this._pendingSet) {
					const type = this._pendingSet
					this._pendingSet = null
					this.closeModal()
					uni.showToast({
						title: `${type === 'voltage' ? '电压' : '电流'}已设置`,
						icon: 'success'
					})
				}
			})

			// 注册设备列表变化回调
			bleService.onDeviceList((devices) => {
				this.bleDevices = devices
			})
		},
		onShow() {
			this.connected = bleService.connected
			this.statusText = bleService.connected ? '已连接' : '点击连接设备'
			if (this.connected) {
				this._updateBleConnectedName()
			}
		},
		onUnload() {
			if (this._setTimeout) {
				clearTimeout(this._setTimeout)
				this._setTimeout = null
			}
			// 页面卸载时停止扫描
			if (this.scanning) {
				bleService.stopScan()
			}
		},
		computed: {
			voltagePercent() {
				return Math.min((this.voltageSet / this.voltageMax) * 100, 100)
			},
			currentPercent() {
				return Math.min((this.currentSet / this.currentMax) * 100, 100)
			},
			calText() {
				const mult = this.calMult || 1.0
				const off = this.calOffset || 0.0
				const offsetStr = off >= 0 ? "+" + off.toFixed(2) : off.toFixed(2)
				return "电压校准: " + mult.toFixed(4) + "x " + offsetStr + "V"
			},
			efficiencyText() {
				const win = this.powerData.W_in || 0
				const wout = this.powerData.W_out || 0
				if (wout <= 0 || win <= 0) return '未输出'
				const pct = Math.min(Math.round((wout / win) * 100), 100)
				return `效率 ${pct}%`
			},
			modalTitle() {
				return this.modalType === 'voltage' ? '设置输出电压' : '设置输出电流'
			},
			modalUnit() {
				return this.modalType === 'voltage' ? 'V' : 'A'
			},
			modalMax() {
				return this.modalType === 'voltage' ? this.voltageMax : this.currentMax
			},
			modalCurrentValue() {
				if (this.modalType === 'voltage') {
					return this.voltageSet.toFixed(2)
				}
				return this.currentSet.toFixed(2)
			}
		},
		methods: {
			/**
			 * 更新已连接设备的显示名称
			 * 优先从本地存储（rename 保存的），其次设备列表，最后用 deviceId 后几位
			 */
			_updateBleConnectedName() {
				const deviceId = bleService.getDeviceId()
				if (!deviceId) {
					this.bleConnectedName = '已连接设备'
					return
				}
				// 1. 优先从本地存储读取（通过 rename 保存的，最可靠）
				try {
					const stored = uni.getStorageSync('ble_last_device')
					if (stored && stored.deviceId === deviceId && stored.name && stored.name.trim().length > 0) {
						this.bleConnectedName = stored.name.trim()
						return
					}
				} catch (e) {}
				// 2. 从设备列表中查找（扫描到的设备名，可能有乱码）
				const found = bleService.getDeviceList().find(d => d.deviceId === deviceId)
				if (found && found.name && found.name.trim().length > 0) {
					this.bleConnectedName = found.name.trim()
					return
				}
				// 3. 用 deviceId 后 8 位
				if (deviceId.length > 8) {
					this.bleConnectedName = '设备(' + deviceId.substring(deviceId.length - 8).toUpperCase() + ')'
				} else {
					this.bleConnectedName = '已连接设备'
				}
			},

			switchTab(tab) {
				const urls = {
					index: '/pages/index/index',
					control: '/pages/control/control',
					settings: '/pages/settings/settings'
				}
				uni.reLaunch({
					url: urls[tab]
				})
			},

			async togglePower() {
				if (!this.connected) {
					uni.showToast({ title: '未连接设备', icon: 'none' })
					return
				}
				try {
					await bleService.setPower(!this.powerOn)
				} catch (err) {
					uni.showToast({ title: '操作失败', icon: 'error' })
				}
			},

			showVoltageInput() {
				if (!this.connected) {
					uni.showToast({ title: '未连接设备', icon: 'none' })
					return
				}
				this.modalType = 'voltage'
				this.modalInputValue = ''
				this.showModal = true
			},

			showCurrentInput() {
				if (!this.connected) {
					uni.showToast({ title: '未连接设备', icon: 'none' })
					return
				}
				this.modalType = 'current'
				this.modalInputValue = ''
				this.showModal = true
			},

			closeModal() {
				this.showModal = false
				this.modalInputValue = ''
			},

			confirmModal() {
				const val = parseFloat(this.modalInputValue)
				const maxVal = this.modalMax
				const unit = this.modalUnit
				if (isNaN(val) || val < 0 || val > maxVal) {
					uni.showToast({ title: `请输入 0-${maxVal}${unit}`, icon: 'none' })
					return
				}
				const type = this.modalType
				const cmd = type === 'voltage' ? 'V_set' : 'I_set'
				// 标记等待确认，关闭弹窗，显示加载中
				this._pendingSet = type
				this.closeModal()
				uni.showLoading({ title: `设置${type === 'voltage' ? '电压' : '电流'}中...` })

				bleService.send({ cmd: 'set', [cmd]: val }).then(() => {
					uni.showToast({ title: '命令已发送，等待确认...', icon: 'none' })
				}).catch((err) => {
					console.error('[首页] 设置命令发送失败:', err)
					this._pendingSet = null
					uni.hideLoading()
					uni.showToast({ title: '发送失败: ' + (err.message || '未知错误'), icon: 'none' })
				})
				// 5秒超时保护
				if (this._setTimeout) clearTimeout(this._setTimeout)
				this._setTimeout = setTimeout(() => {
					if (this._pendingSet) {
						this._pendingSet = null
						uni.hideLoading()
						uni.showToast({ title: '设置超时，未收到确认', icon: 'none' })
					}
				}, 5000)
			},

			/**
			 * 切换预设编辑模式
			 */
			togglePresetEdit() {
				this.presetEditing = !this.presetEditing
			},

			/**
			 * 编辑模式下输入预设值
			 */
			onPresetEditInput(index, field, event) {
				const val = parseFloat(event.detail.value)
				if (!isNaN(val)) {
					if (field === 'voltage' && val >= 0 && val <= this.voltageMax) {
						this.presets[index].voltage = val
					} else if (field === 'current' && val >= 0 && val <= this.currentMax) {
						this.presets[index].current = val
					}
				}
			},

			/**
			 * 点击预设 - 弹出确认应用弹窗
			 */
			confirmApplyPreset(preset) {
				if (!this.connected) {
					uni.showToast({ title: '未连接设备', icon: 'none' })
					return
				}
				this.presetConfirmData = { ...preset }
				this.showPresetConfirm = true
			},

			/**
			 * 关闭预设确认弹窗
			 */
			closePresetConfirm() {
				this.showPresetConfirm = false
				this.presetConfirmData = { label: '', voltage: 0, current: 0 }
			},

			/**
			 * 确认应用预设到设备
			 */
			async doApplyPreset() {
				const { voltage, current } = this.presetConfirmData
				this.closePresetConfirm()
				this.voltageSet = voltage
				this.currentSet = current
				try {
					await bleService.setVoltage(voltage)
					await bleService.setCurrent(current)
					uni.showToast({ title: `已设为 ${voltage}V/${current}A`, icon: 'success' })
				} catch (err) {
					uni.showToast({ title: '设定失败', icon: 'error' })
				}
			},
				goCalibration() {
					this.closeModal()
					uni.navigateTo({
						url: '/pages/calibration/calibration'
					})
				},

			async clearFaults() {
				if (!this.connected) {
					uni.showToast({ title: '未连接设备', icon: 'none' })
					return
				}
				try {
					await bleService.clearFaults()
					uni.showToast({ title: '已清除故障', icon: 'success' })
				} catch (err) {
					uni.showToast({ title: '操作失败', icon: 'error' })
				}
			},

			// ===== 蓝牙设备列表相关方法 =====

			/**
			 * 显示蓝牙设备列表弹窗
			 */
			showBleDeviceList() {
				this.showBleModal = true
				// 如果已连接，直接显示已连接信息
				if (this.connected) {
					this._updateBleConnectedName()
					return
				}
				// 未连接时，初始化蓝牙并开始扫描
				bleService.init()
				// 获取已有设备列表
				this.bleDevices = bleService.getDeviceList()
				this.scanning = bleService.scanning
				// 如果不在扫描中，自动开始扫描
				if (!this.scanning) {
					this.startBleScan()
				}
			},

			/**
			 * 关闭蓝牙设备列表弹窗
			 */
			closeBleModal() {
				this.showBleModal = false
				// 关闭弹窗时停止扫描
				if (this.scanning) {
					this.stopBleScan()
				}
			},

			/**
			 * 开始扫描 BLE 设备
			 */
			startBleScan() {
				this.scanning = true
				this.bleDevices = []
				bleService.startScan()
			},

			/**
			 * 停止扫描 BLE 设备
			 */
			stopBleScan() {
				this.scanning = false
				bleService.stopScan()
			},

			/**
			 * 连接选中的设备
			 */
			async connectDevice(device) {
				if (this.scanning) {
					this.stopBleScan()
				}
				uni.showLoading({ title: '连接中...' })
				try {
					await bleService.connect(device.deviceId, device.name)
					this.connected = true
					this.bleConnectedName = device.name
					this.closeBleModal()
					uni.hideLoading()
					uni.showToast({ title: '连接成功', icon: 'success' })
				} catch (err) {
					uni.hideLoading()
					uni.showToast({ title: '连接失败: ' + (err.message || '未知错误'), icon: 'none' })
				}
			},

			/**
				* 断开当前设备连接
			 */
			disconnectDevice() {
				uni.showModal({
					title: '断开连接',
					content: '确定要断开当前设备连接吗？',
					success: (res) => {
						if (res.confirm) {
							bleService.disconnect()
							this.connected = false
							this.bleConnectedName = ''
							this.closeBleModal()
							uni.showToast({ title: '已断开连接', icon: 'success' })
						}
					}
				})
			},

			/**
			 * 根据 RSSI 返回信号强度样式
			 */
			rssiClass(rssi) {
				if (rssi >= -60) return 'strong'
				if (rssi >= -80) return 'medium'
				return 'weak'
			}
		}
	}
</script>

<style>
	page {
		background-color: #0f0f23;
		color: #e0e0e0;
		font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
	}

	.container {
		padding: 20rpx 30rpx 120rpx;
		min-height: 100vh;
	}

	/* 信息网格 */
	.info-grid {
		display: flex;
		gap: 20rpx;
		margin-bottom: 20rpx;
	}

	.info-card {
		flex: 1;
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 16rpx;
		padding: 24rpx;
		border: 1px solid #2a2a5e;
	}

	.power-card {
		border-color: #ffd54f;
	}

	.power-card .info-value {
		color: #ffd54f;
	}

	.info-label {
		font-size: 22rpx;
		color: #888;
		display: block;
		margin-bottom: 8rpx;
	}

	.info-value {
		font-size: 36rpx;
		font-weight: 600;
		color: #e0e0e0;
	}

	.info-unit {
		font-size: 22rpx;
		font-weight: 400;
		color: #666;
	}

	/* 顶部栏 */
	.header-bar {
		display: flex;
		justify-content: space-between;
		align-items: center;
		margin-bottom: 24rpx;
		padding: 10rpx 0;
	}

	.header-title {
		font-size: 36rpx;
		font-weight: 700;
		color: #fff;
	}

	.header-actions {
		display: flex;
		align-items: center;
		gap: 16rpx;
	}

	.header-btn {
		display: flex;
		align-items: center;
		gap: 6rpx;
		padding: 8rpx 20rpx;
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 30rpx;
		border: 1px solid #2a2a5e;
	}

	.header-btn-icon {
		font-size: 28rpx;
	}

	.header-btn-label {
		font-size: 24rpx;
		color: #888;
	}

	/* 电源状态 */
	.power-status-section {
		position: relative;
		display: flex;
		align-items: center;
		gap: 30rpx;
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 20rpx;
		padding: 30rpx;
		border: 1px solid #2a2a5e;
		margin-bottom: 30rpx;
		overflow: hidden;
	}

	/* 效率标签 - 右上角 */
	.power-efficiency-badge {
		position: absolute;
		top: 12rpx;
		right: 16rpx;
		padding: 4rpx 14rpx;
		border-radius: 20rpx;
		background: rgba(255, 255, 255, 0.04);
		border: 1px solid rgba(255, 255, 255, 0.06);
		z-index: 2;
	}

	.power-efficiency-badge.active {
		background: rgba(255, 213, 79, 0.12);
		border-color: rgba(255, 213, 79, 0.3);
	}

	.power-efficiency-text {
		font-size: 20rpx;
		color: #666;
		font-weight: 500;
	}

	.power-efficiency-badge.active .power-efficiency-text {
		color: #ffd54f;
	}

	.power-toggle {
		width: 120rpx;
		height: 120rpx;
		border-radius: 50%;
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
		background: #2a2a5e;
		border: 2px solid #3a3a6e;
		transition: all 0.3s;
		position: relative;
	}

	/* 运行中呼吸光晕动画 */
	.power-toggle.on {
		background: rgba(0, 230, 118, 0.15);
		border-color: #00e676;
		box-shadow: 0 0 20rpx rgba(0, 230, 118, 0.3);
		animation: powerBreath 2s ease-in-out infinite;
	}

	/* 脉冲光环 */
	.power-toggle.on::before {
		content: '';
		position: absolute;
		top: -8rpx;
		left: -8rpx;
		right: -8rpx;
		bottom: -8rpx;
		border-radius: 50%;
		border: 2px solid rgba(0, 230, 118, 0.3);
		animation: powerPulse 2s ease-in-out infinite;
	}

	@keyframes powerBreath {
		0%, 100% {
			box-shadow: 0 0 20rpx rgba(0, 230, 118, 0.3);
			transform: scale(1);
		}
		50% {
			box-shadow: 0 0 40rpx rgba(0, 230, 118, 0.5);
			transform: scale(1.05);
		}
	}

	@keyframes powerPulse {
		0%, 100% {
			opacity: 0.4;
			transform: scale(1);
		}
		50% {
			opacity: 0.8;
			transform: scale(1.1);
		}
	}

	.power-icon {
		font-size: 40rpx;
		position: relative;
		z-index: 1;
	}

	.power-label {
		font-size: 18rpx;
		color: #888;
		margin-top: 4rpx;
		position: relative;
		z-index: 1;
	}

	.power-toggle.on .power-label {
		color: #00e676;
	}

	.power-info {
		flex: 1;
	}

	.power-info-label {
		font-size: 24rpx;
		color: #888;
		display: block;
		margin-bottom: 8rpx;
	}

	.power-info-row {
		display: flex;
		align-items: flex-end;
		gap: 12rpx;
		height: 80rpx;
	}

	.power-info-value {
		font-size: 36rpx;
		font-weight: 600;
		color: #666;
		line-height: 1;
	}

	.power-info-value.on {
		color: #00e676;
	}

	/* 能量流光条 - 在电源状态信息底部 */
	.energy-flow {
		margin-top: 12rpx;
		width: 100%;
		height: 6rpx;
		border-radius: 3rpx;
		overflow: hidden;
		background: rgba(255, 255, 255, 0.04);
		position: relative;
	}

	.energy-flow.active {
		background: rgba(0, 230, 118, 0.08);
	}

	.energy-flow-track {
		position: relative;
		width: 100%;
		height: 100%;
	}

	.energy-flow-bar {
		position: absolute;
		top: 0;
		left: -40%;
		width: 40%;
		height: 100%;
		border-radius: 3rpx;
		background: linear-gradient(90deg, transparent, #00e676, #00e676, transparent);
		opacity: 0;
		transition: opacity 0.3s;
	}

	.energy-flow.active .energy-flow-bar {
		opacity: 1;
		animation: energyFlow 1.2s ease-in-out infinite;
	}

	/* 流光光晕 */
	.energy-flow.active::after {
		content: '';
		position: absolute;
		top: -4rpx;
		left: 0;
		right: 0;
		height: 14rpx;
		background: radial-gradient(ellipse at center, rgba(0, 230, 118, 0.15) 0%, transparent 70%);
		animation: energyGlow 1.2s ease-in-out infinite;
	}

	@keyframes energyFlow {
		0% { left: -40%; }
		100% { left: 100%; }
	}

	@keyframes energyGlow {
		0% { opacity: 0.3; transform: scaleX(0.8); }
		50% { opacity: 0.8; transform: scaleX(1.2); }
		100% { opacity: 0.3; transform: scaleX(0.8); }
	}

	/* 电压/电流并排行 */
	.meter-row {
		display: flex;
		gap: 20rpx;
		margin-bottom: 20rpx;
	}

	/* 仪表盘卡片 */
	.meter-card {
		flex: 1;
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 20rpx;
		padding: 24rpx;
		border: 1px solid #2a2a5e;
	}

	.voltage-card {
		border-color: #4fc3f7;
	}

	.current-card {
		border-color: #ff8a65;
	}

	.meter-label {
		font-size: 24rpx;
		color: #888;
		margin-bottom: 12rpx;
	}

	.tap-hint {
		font-size: 20rpx;
		color: #4fc3f7;
		margin-left: 8rpx;
		opacity: 0.7;
	}

	.current-card .tap-hint {
		color: #ff8a65;
	}

	.meter-value {
		display: flex;
		align-items: baseline;
		margin-bottom: 16rpx;
	}

	.value-number {
		font-size: 64rpx;
		font-weight: 700;
		color: #fff;
		line-height: 1;
	}

	.voltage-card .value-number {
		color: #4fc3f7;
	}

	.current-card .value-number {
		color: #ff8a65;
	}

	.value-unit {
		font-size: 28rpx;
		color: #888;
		margin-left: 8rpx;
	}

	.meter-bar {
		height: 8rpx;
		background-color: #2a2a5e;
		border-radius: 4rpx;
		overflow: hidden;
	}

	.meter-bar-fill {
		height: 100%;
		border-radius: 4rpx;
		transition: width 0.5s ease;
	}

	.voltage-fill {
		background: linear-gradient(90deg, #4fc3f7, #0288d1);
	}

	.current-fill {
		background: linear-gradient(90deg, #ff8a65, #d84315);
	}

	/* 预设 */
	.preset-section {
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 20rpx;
		padding: 30rpx;
		border: 1px solid #2a2a5e;
		margin-bottom: 30rpx;
	}

	.preset-header {
		display: flex;
		justify-content: space-between;
		align-items: center;
		margin-bottom: 20rpx;
	}

	.preset-title {
		font-size: 26rpx;
		color: #888;
	}

	.preset-header-actions {
		display: flex;
		align-items: center;
		gap: 12rpx;
	}

	.preset-edit-btn {
		font-size: 22rpx;
		color: #4fc3f7;
		padding: 6rpx 16rpx;
		border-radius: 20rpx;
		border: 1px solid rgba(79, 195, 247, 0.2);
		background: rgba(79, 195, 247, 0.06);
	}

	.preset-edit-btn.editing {
		color: #ffd54f;
		border-color: rgba(255, 213, 79, 0.3);
		background: rgba(255, 213, 79, 0.1);
	}

	.preset-grid {
		display: grid;
		grid-template-columns: repeat(3, 1fr);
		gap: 16rpx;
	}

	.preset-btn {
		background: #2a2a5e;
		border-radius: 12rpx;
		padding: 20rpx 12rpx;
		text-align: center;
		border: 1px solid #3a3a6e;
	}

	.preset-label {
		font-size: 24rpx;
		color: #e0e0e0;
		display: block;
		margin-bottom: 6rpx;
	}

	.preset-detail {
		font-size: 20rpx;
		color: #888;
	}

	/* 预设编辑模式 - 按钮变成输入框样式 */
	.preset-btn-editing {
		padding: 14rpx 10rpx !important;
		border-color: rgba(79, 195, 247, 0.3) !important;
		background: rgba(79, 195, 247, 0.04) !important;
	}

	.preset-edit-label-input {
		width: 100%;
		height: 48rpx;
		background: rgba(255, 255, 255, 0.04);
		border: 1px solid #3a3a6e;
		border-radius: 8rpx;
		padding: 0 10rpx;
		font-size: 22rpx;
		color: #e0e0e0;
		text-align: center;
		box-sizing: border-box;
		margin-bottom: 8rpx;
	}

	.preset-edit-inline {
		display: flex;
		align-items: center;
		justify-content: center;
		gap: 6rpx;
	}

	.preset-edit-inline-input {
		width: 72rpx;
		height: 48rpx;
		background: rgba(255, 255, 255, 0.04);
		border: 1px solid #3a3a6e;
		border-radius: 8rpx;
		padding: 0 6rpx;
		font-size: 22rpx;
		color: #4fc3f7;
		text-align: center;
		box-sizing: border-box;
	}

	.preset-edit-slash {
		font-size: 22rpx;
		color: #555;
	}

	/* 预设确认应用弹窗 */
	.preset-confirm-dialog {
		width: 520rpx !important;
	}

	.preset-confirm-icon {
		font-size: 64rpx;
		text-align: center;
		display: block;
		margin-bottom: 16rpx;
	}

	.preset-confirm-label {
		font-size: 32rpx;
		color: #e0e0e0;
		font-weight: 600;
		text-align: center;
		display: block;
		margin-bottom: 24rpx;
	}

	.preset-confirm-values {
		display: flex;
		align-items: center;
		justify-content: center;
		gap: 30rpx;
		background: rgba(255, 255, 255, 0.03);
		border-radius: 16rpx;
		padding: 24rpx;
	}

	.preset-confirm-item {
		text-align: center;
	}

	.preset-confirm-item-label {
		font-size: 22rpx;
		color: #888;
		display: block;
		margin-bottom: 8rpx;
	}

	.preset-confirm-item-value {
		font-size: 44rpx;
		font-weight: 700;
	}

	.preset-confirm-item-value.voltage {
		color: #4fc3f7;
	}

	.preset-confirm-item-value.current {
		color: #ffd54f;
	}

	.preset-confirm-item-unit {
		font-size: 24rpx;
		font-weight: 400;
		opacity: 0.6;
	}

	.preset-confirm-divider {
		width: 1px;
		height: 60rpx;
		background: #2a2a5e;
	}

	.preset-confirm-footer {
		padding: 20rpx 36rpx 32rpx !important;
	}

	/* 状态标签 */
	.section-title {
		font-size: 26rpx;
		color: #666;
		margin: 20rpx 0 16rpx;
		padding-left: 16rpx;
		border-left: 6rpx solid #4fc3f7;
	}

	.status-grid {
		display: flex;
		flex-wrap: wrap;
		gap: 16rpx;
		margin-bottom: 30rpx;
	}

	.status-chip {
		padding: 12rpx 24rpx;
		border-radius: 30rpx;
		font-size: 24rpx;
		background-color: #2a2a5e;
		color: #888;
		border: 1px solid #3a3a6e;
	}

	.status-chip.ok {
		background-color: rgba(0, 230, 118, 0.1);
		color: #00e676;
		border-color: rgba(0, 230, 118, 0.3);
	}

	/* 底部导航 */
	.bottom-nav {
		position: fixed;
		bottom: 0;
		left: 0;
		right: 0;
		display: flex;
		background: #1a1a2e;
		border-top: 1px solid #2a2a5e;
		padding: 12rpx 0;
		padding-bottom: calc(12rpx + env(safe-area-inset-bottom));
	}

	.nav-item {
		flex: 1;
		display: flex;
		flex-direction: column;
		align-items: center;
		padding: 8rpx 0;
	}

	.nav-item.active .nav-label {
		color: #4fc3f7;
	}

	.nav-icon {
		font-size: 44rpx;
		margin-bottom: 4rpx;
	}

	.nav-label {
		font-size: 22rpx;
		color: #666;
	}

	/* 底部导航蓝牙状态 */
	.nav-ble {
		display: flex;
		align-items: center;
		justify-content: center;
		padding: 8rpx 20rpx 8rpx 0;
	}

	.nav-ble-dot {
		width: 14rpx;
		height: 14rpx;
		border-radius: 50%;
		background-color: #444;
		transition: all 0.3s;
	}

	.nav-ble-dot.active {
		background-color: #00e676;
		box-shadow: 0 0 10rpx #00e676;
	}

	.nav-ble.connected {
		opacity: 1;
	}

	/* ===== 自定义弹窗样式 ===== */
	.modal-overlay {
		position: fixed;
		top: 0;
		left: 0;
		right: 0;
		bottom: 0;
		background: rgba(0, 0, 0, 0.6);
		display: flex;
		align-items: flex-start;
		justify-content: center;
		z-index: 999;
		backdrop-filter: blur(4px);
		padding-top: 120rpx;
	}

	.modal-dialog {
		width: 580rpx;
		background: #1a1a2e;
		border-radius: 24rpx;
		overflow: hidden;
		border: 1px solid rgba(255, 255, 255, 0.08);
		box-shadow: 0 20rpx 60rpx rgba(0, 0, 0, 0.5);
	}

	.modal-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
		padding: 32rpx 36rpx 20rpx;
	}

	.modal-title {
		font-size: 32rpx;
		font-weight: 600;
		color: #fff;
	}

	.modal-close {
		font-size: 28rpx;
		color: #666;
		padding: 8rpx;
		width: 48rpx;
		height: 48rpx;
		text-align: center;
		line-height: 48rpx;
		border-radius: 50%;
		background: rgba(255, 255, 255, 0.05);
	}

	.modal-close:active {
		background: rgba(255, 255, 255, 0.1);
	}

	.modal-body {
		padding: 20rpx 36rpx 32rpx;
	}

	.modal-current {
		display: flex;
		align-items: baseline;
		justify-content: center;
		padding: 24rpx 0;
		margin-bottom: 24rpx;
		background: rgba(255, 255, 255, 0.03);
		border-radius: 16rpx;
	}

	.modal-current-label {
		font-size: 24rpx;
		color: #888;
		margin-right: 16rpx;
	}

	.modal-current-value {
		font-size: 56rpx;
		font-weight: 700;
		color: #4fc3f7;
	}

	.modal-current-value.current {
		color: #ff8a65;
	}

	.modal-current-unit {
		font-size: 28rpx;
		color: #888;
		margin-left: 8rpx;
	}

	.modal-input-group {
		margin-bottom: 12rpx;
	}

	.modal-input-label {
		font-size: 24rpx;
		color: #888;
		margin-bottom: 12rpx;
		display: block;
	}

	.modal-input-row {
		display: flex;
		align-items: center;
		background: rgba(255, 255, 255, 0.05);
		border: 1px solid rgba(255, 255, 255, 0.1);
		border-radius: 12rpx;
		padding: 0 24rpx;
	}

	.modal-input-row:focus-within {
		border-color: #4fc3f7;
	}

	.modal-input {
		flex: 1;
		height: 88rpx;
		font-size: 36rpx;
		color: #fff;
		background: transparent;
		border: none;
		outline: none;
	}

	.modal-input-unit {
		font-size: 28rpx;
		color: #888;
		margin-left: 8rpx;
	}

	.modal-range {
		margin-top: 12rpx;
	}

	.modal-range-text {
		font-size: 22rpx;
		color: #666;
	}

	.modal-footer {
		display: flex;
		border-top: 1px solid rgba(255, 255, 255, 0.06);
	}

	.modal-btn {
		flex: 1;
		height: 96rpx;
		display: flex;
		align-items: center;
		justify-content: center;
		font-size: 30rpx;
		font-weight: 500;
	}

	.modal-btn:active {
		opacity: 0.7;
	}

	.modal-btn-cancel {
		color: #888;
		border-right: 1px solid rgba(255, 255, 255, 0.06);
	}

	.modal-btn-confirm {
		color: #4fc3f7;
	}

	.modal-btn-confirm.current {
		color: #ff8a65;
	}
	/* ===== 蓝牙设备列表弹窗样式 ===== */
	.ble-modal-overlay {
		position: fixed;
		top: 0;
		left: 0;
		right: 0;
		bottom: 0;
		background: rgba(0, 0, 0, 0.6);
		display: flex;
		align-items: center;
		justify-content: center;
		z-index: 999;
		backdrop-filter: blur(4px);
	}

	.ble-modal-dialog {
		width: 620rpx;
		max-height: 75vh;
		background: #1a1a2e;
		border-radius: 24rpx;
		overflow: hidden;
		border: 1px solid rgba(255, 255, 255, 0.08);
		box-shadow: 0 20rpx 60rpx rgba(0, 0, 0, 0.5);
		display: flex;
		flex-direction: column;
	}

	.ble-modal-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
		padding: 32rpx 36rpx 20rpx;
	}

	.ble-modal-title {
		font-size: 32rpx;
		font-weight: 600;
		color: #fff;
	}

	.ble-modal-close {
		font-size: 28rpx;
		color: #666;
		padding: 8rpx;
		width: 48rpx;
		height: 48rpx;
		text-align: center;
		line-height: 48rpx;
		border-radius: 50%;
		background: rgba(255, 255, 255, 0.05);
	}

	.ble-modal-close:active {
		background: rgba(255, 255, 255, 0.1);
	}

	/* 已连接信息 */
	.ble-connected-info {
		padding: 20rpx 36rpx;
		display: flex;
		align-items: center;
		justify-content: space-between;
	}

	.ble-connected-device {
		display: flex;
		align-items: center;
		gap: 16rpx;
	}

	.ble-connected-icon {
		font-size: 40rpx;
	}

	.ble-connected-detail {
		display: flex;
		flex-direction: column;
	}

	.ble-connected-name {
		font-size: 28rpx;
		color: #e0e0e0;
		font-weight: 500;
	}

	.ble-connected-status {
		font-size: 22rpx;
		color: #00e676;
		margin-top: 4rpx;
	}

	.ble-btn-disconnect {
		padding: 12rpx 28rpx;
		border-radius: 30rpx;
		font-size: 24rpx;
		color: #ff5252;
		background: rgba(255, 82, 82, 0.1);
		border: 1px solid rgba(255, 82, 82, 0.3);
	}

	.ble-btn-disconnect:active {
		background: rgba(255, 82, 82, 0.2);
	}

	/* 扫描状态 */
	.ble-scan-status {
		padding: 16rpx 36rpx;
	}

	.ble-scanning-text {
		font-size: 24rpx;
		color: #4fc3f7;
		animation: pulse 1.5s ease-in-out infinite;
	}

	.ble-scan-idle {
		font-size: 24rpx;
		color: #666;
	}

	@keyframes pulse {
		0%, 100% { opacity: 1; }
		50% { opacity: 0.5; }
	}

	/* 设备列表 */
	.ble-device-list {
		padding: 0 36rpx;
		max-height: 400rpx;
		overflow-y: auto;
		flex: 1;
	}

	.ble-device-item {
		display: flex;
		align-items: center;
		gap: 16rpx;
		padding: 24rpx 16rpx;
		border-bottom: 1px solid rgba(255, 255, 255, 0.05);
		border-radius: 12rpx;
		transition: background 0.2s;
	}

	.ble-device-item:active {
		background: rgba(79, 195, 247, 0.08);
	}

	.ble-device-item:last-child {
		border-bottom: none;
	}

	.ble-device-icon {
		font-size: 36rpx;
		width: 48rpx;
		text-align: center;
	}

	.ble-device-info {
		flex: 1;
		display: flex;
		flex-direction: column;
		min-width: 0;
	}

	.ble-device-name {
		font-size: 28rpx;
		color: #e0e0e0;
		font-weight: 500;
		overflow: hidden;
		text-overflow: ellipsis;
		white-space: nowrap;
	}

	.ble-device-id {
		font-size: 20rpx;
		color: #555;
		margin-top: 4rpx;
		overflow: hidden;
		text-overflow: ellipsis;
		white-space: nowrap;
	}

	.ble-device-rssi {
		display: flex;
		align-items: center;
	}

	.ble-rssi-bar {
		font-size: 22rpx;
		padding: 4rpx 14rpx;
		border-radius: 20rpx;
		background: rgba(255, 255, 255, 0.05);
	}

	.ble-rssi-bar.strong {
		color: #00e676;
		background: rgba(0, 230, 118, 0.1);
	}

	.ble-rssi-bar.medium {
		color: #ffd54f;
		background: rgba(255, 213, 79, 0.1);
	}

	.ble-rssi-bar.weak {
		color: #ff5252;
		background: rgba(255, 82, 82, 0.1);
	}

	/* 空状态 */
	.ble-empty {
		padding: 60rpx 0;
		text-align: center;
	}

	.ble-empty-text {
		font-size: 26rpx;
		color: #555;
	}

	/* 底部按钮 */
	.ble-modal-footer {
		padding: 20rpx 36rpx 32rpx;
	}

	.ble-btn {
		height: 88rpx;
		display: flex;
		align-items: center;
		justify-content: center;
		border-radius: 44rpx;
		font-size: 30rpx;
		font-weight: 500;
	}

	.ble-btn:active {
		opacity: 0.7;
	}

	.ble-btn-scan {
		background: linear-gradient(135deg, #4fc3f7, #0288d1);
		color: #fff;
	}

	.ble-btn-stop {
		background: rgba(255, 255, 255, 0.08);
		color: #ff5252;
		border: 1px solid rgba(255, 82, 82, 0.3);
	}

	/* ===== 电压设置弹窗 - 校准入口 ===== */
	.modal-cal-section {
		margin-top: 20rpx;
	}

	.modal-cal-divider {
		height: 1px;
		background: rgba(255, 255, 255, 0.06);
		margin-bottom: 20rpx;
	}

	.modal-cal-btn {
		display: flex;
		align-items: center;
		padding: 24rpx 20rpx;
		background: rgba(255, 143, 0, 0.08);
		border-radius: 16rpx;
		border: 1px solid rgba(255, 143, 0, 0.2);
	}

	.modal-cal-btn:active {
		background: rgba(255, 143, 0, 0.15);
	}

	.modal-cal-icon {
		font-size: 36rpx;
		margin-right: 16rpx;
	}

	.modal-cal-text {
		flex: 1;
		font-size: 26rpx;
		color: #ffb74d;
		font-weight: 500;
	}

	.modal-cal-arrow {
		font-size: 36rpx;
		color: #ffb74d;
		opacity: 0.6;
	}

</style>
