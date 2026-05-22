<template>
	<view class="container">

		<!-- 标签切换 (连接后显示) -->
		<view v-if="connected" class="tab-bar">
			<view class="tab-item" :class="{ active: activeTab === 'control' }" @tap="activeTab = 'control'">
				<text class="tab-icon">🎛️</text>
				<text class="tab-label">控制</text>
			</view>
			<view class="tab-item" :class="{ active: activeTab === 'device' }" @tap="activeTab = 'device'">
				<text class="tab-icon">📡</text>
				<text class="tab-label">设备</text>
			</view>
		</view>

		<!-- 设备列表 -->
		<view class="scan-section" v-show="!connected || activeTab === 'device'">
			<view class="scan-header">
				<text class="scan-title">蓝牙设备</text>
				<button class="scan-btn" :disabled="scanning" @tap="startScan">
					{{ scanning ? '扫描中...' : '扫描设备' }}
				</button>
			</view>

			<view v-if="scanning" class="scanning-indicator">
				<text class="scanning-text">正在搜索蓝牙设备...</text>
			</view>

			<view v-if="devices.length === 0 && !scanning" class="empty-state">
				<text class="empty-icon">📡</text>
				<text class="empty-text">点击"扫描设备"搜索 Dell-PSU-Controller</text>
			</view>

			<view class="device-list">
				<view
					v-for="device in devices"
					:key="device.deviceId"
					class="device-item"
					:class="{ connected: connected && device.deviceId === bleService._deviceId }"
					@tap="connectDevice(device)"
				>
					<view class="device-icon">🔌</view>
					<view class="device-info">
						<text class="device-name">{{ displayName(device) }}</text>
						<text class="device-id">{{ device.deviceId }}</text>
						<text class="device-rssi" v-if="device.RSSI">
							信号: {{ device.RSSI }} dBm
						</text>
					</view>
					<text class="device-connect" v-if="connected && device.deviceId === bleService._deviceId">已连接 ✓</text>
					<text class="device-connect" v-else>连接 ›</text>
				</view>
			</view>
		</view>

		<!-- 控制面板 (已连接时显示) -->
		<view v-if="connected" class="control-panel" v-show="activeTab === 'control'">
			<!-- 电源开关 -->
			<view class="power-section">
				<view class="power-toggle" :class="{ on: powerOn }" @tap="togglePower">
					<text class="power-icon">{{ powerOn ? '⏻' : '⏻' }}</text>
					<text class="power-label">{{ powerOn ? '开机' : '关机' }}</text>
				</view>
				<view class="power-status">
					<text class="power-status-label">电源状态</text>
					<text class="power-status-value" :class="{ on: powerOn }">
						{{ powerOn ? '运行中' : '已关闭' }}
					</text>
				</view>
			</view>

			<!-- 电压控制 -->
			<view class="control-section">
				<view class="control-header">
					<text class="control-label">输出电压</text>
					<text class="control-value">{{ voltageSet.toFixed(2) }} V</text>
				</view>
				<slider
					:min="0"
					:max="voltageMax * 100"
					:step="10"
					:value="voltageSet * 100"
					@change="onVoltageChange"
					@changing="onVoltageChanging"
					backgroundColor="#2a2a5e"
					activeColor="#4fc3f7"
					blockColor="#4fc3f7"
					blockSize="28"
				/>
				<view class="slider-range">
					<text>0.00V</text>
					<text>{{ voltageMax.toFixed(1) }}V</text>
				</view>
			</view>

			<!-- 电流控制 -->
			<view class="control-section">
				<view class="control-header">
					<text class="control-label">输出电流</text>
					<text class="control-value current">{{ currentSet.toFixed(2) }} A</text>
				</view>
				<slider
					:min="0"
					:max="currentMax * 100"
					:step="50"
					:value="currentSet * 100"
					@change="onCurrentChange"
					@changing="onCurrentChanging"
					backgroundColor="#2a2a5e"
					activeColor="#ff8a65"
					blockColor="#ff8a65"
					blockSize="28"
				/>
				<view class="slider-range">
					<text>0.00A</text>
					<text>{{ currentMax.toFixed(1) }}A</text>
				</view>
			</view>

			<!-- 快速预设 -->
			<view class="preset-section">
				<text class="preset-title">快速预设</text>
				<view class="preset-grid">
					<view
						v-for="preset in presets"
						:key="preset.label"
						class="preset-btn"
						@tap="applyPreset(preset)"
					>
						<text class="preset-label">{{ preset.label }}</text>
						<text class="preset-detail">{{ preset.voltage }}V / {{ preset.current }}A</text>
					</view>
				</view>
			</view>

			<!-- 操作按钮 -->
			<view class="action-buttons">
				<button class="action-btn danger" @tap="clearFaults">清除故障</button>
				<button class="action-btn secondary" @tap="disconnect">断开连接</button>
			</view>
		</view>

		<!-- 底部导航 -->
		<view class="bottom-nav">
			<view class="nav-item" @tap="switchTab('index')">
				<text class="nav-icon">📊</text>
				<text class="nav-label">监控</text>
			</view>
			<view class="nav-item active" @tap="switchTab('control')">
				<text class="nav-icon">🎛️</text>
				<text class="nav-label">控制</text>
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
				bleService,  // 暴露给模板使用
				connected: false,
				statusText: '未连接',
				scanning: false,
				devices: [],
				activeTab: 'control',  // 'control' 或 'device'
				powerOn: false,
				voltageSet: 0,
				currentSet: 0,
				voltageMax: 12.0,
				currentMax: 60.0,
				presets: [
					{ label: '低压小电流', voltage: 3.3, current: 1.0 },
					{ label: '5V 2A', voltage: 5.0, current: 2.0 },
					{ label: '12V 5A', voltage: 12.0, current: 5.0 },
					{ label: '12V 10A', voltage: 12.0, current: 10.0 },
					{ label: '12V 20A', voltage: 12.0, current: 20.0 },
					{ label: '满载', voltage: 12.0, current: 60.0 }
				]
			}
		},
		onLoad() {
			// 蓝牙已在 App.vue onLaunch 中全局初始化，此处不再重复调用

			bleService.onStatus((status) => {
				this.statusText = status
				// 同步连接状态, 确保 UI 正确显示/隐藏控制面板
				this.connected = bleService.connected
			})

			bleService.onData((data) => {
				// 根据 ESP32 返回的实际数据更新 UI
				// ESP32 的 power_on 字段基于 PMBus V_out 读取的真实状态
				if (data.power_on !== undefined) {
					this.powerOn = data.power_on === 1
				}
				if (data.V_out !== undefined) {
					this.voltageSet = data.V_out
				}
				if (data.I_out !== undefined) {
					this.currentSet = data.I_out
				}
				// 处理设置命令的确认响应，同步电源状态
				if (data.ack === 'ok' && data.power !== undefined) {
					this.powerOn = data.power === 1
				}
			})

			// 同步连接状态（连接状态变化由 ble-service.js 全局监听处理）
			this.connected = bleService.connected
			this.statusText = bleService.connected ? '已连接' : '未连接'
			if (bleService.connected && this.devices.length === 0) {
				this.devices.push({
					deviceId: bleService._deviceId,
					name: 'Dell-PSU-Controller',
					RSSI: 0
				})
			}
		},
		onUnload() {
			bleService.stopScan()
		},
		methods: {
			displayName(device) {
				if (!device.name || device.name === 'N/A' || device.name === 'null') {
					return '未知设备'
				}
				return device.name
			},

			startScan() {
				this.scanning = true
				this.devices = []

				bleService.startScan((device) => {
					// 去重
					const exists = this.devices.find(d => d.deviceId === device.deviceId)
					if (!exists) {
						this.devices.push(device)
					}
				})

				// 5 秒后自动停止
				setTimeout(() => {
					bleService.stopScan()
					this.scanning = false
				}, 5000)
			},

			async connectDevice(device) {
				uni.showLoading({ title: '连接中...' })
				try {
					bleService.stopScan()
					this.scanning = false
					await bleService.connect(device.deviceId)
					this.connected = true
					this.statusText = '已连接'
					// 确保设备在列表中
					const exists = this.devices.find(d => d.deviceId === device.deviceId)
					if (!exists) {
						this.devices.push(device)
					}
					uni.hideLoading()
					uni.showToast({ title: '连接成功', icon: 'success' })
					// 连接成功后主动请求数据, 确保 UI 立即更新
					// ESP32 端也会自动推送, 但主动请求更可靠
					setTimeout(() => {
						bleService.getData().catch(err => {
							console.error('[Control] 首次获取数据失败:', err)
						})
					}, 1000)
				} catch (err) {
					uni.hideLoading()
					uni.showToast({ title: '连接失败', icon: 'error' })
				}
			},

			async togglePower() {
					try {
						// 发送电源切换命令，不立即翻转状态
						// 等待 ESP32 返回的确认响应（onData 中处理 ack 响应）来更新 UI
						await bleService.setPower(!this.powerOn)
						// 不立即翻转 this.powerOn，等待 ESP32 返回的实际 power_on 状态
						// 实际状态会通过 onData 回调中的 data.power_on 或 data.ack.power 更新
					} catch (err) {
						uni.showToast({ title: '操作失败', icon: 'error' })
					}
				},

			onVoltageChange(e) {
				const value = e.detail.value / 100
				this.voltageSet = value
				this._sendVoltage(value)
			},

			onVoltageChanging(e) {
				this.voltageSet = e.detail.value / 100
			},

			onCurrentChange(e) {
				const value = e.detail.value / 100
				this.currentSet = value
				this._sendCurrent(value)
			},

			onCurrentChanging(e) {
				this.currentSet = e.detail.value / 100
			},

			async _sendVoltage(value) {
				try {
					await bleService.setVoltage(value)
				} catch (err) {
					console.error('设定电压失败:', err)
				}
			},

			async _sendCurrent(value) {
				try {
					await bleService.setCurrent(value)
				} catch (err) {
					console.error('设定电流失败:', err)
				}
			},

			async applyPreset(preset) {
				this.voltageSet = preset.voltage
				this.currentSet = preset.current
				try {
					await bleService.setVoltage(preset.voltage)
					await bleService.setCurrent(preset.current)
					uni.showToast({ title: `已设为 ${preset.voltage}V/${preset.current}A`, icon: 'none' })
				} catch (err) {
					uni.showToast({ title: '设定失败', icon: 'error' })
				}
			},

			async clearFaults() {
				try {
					await bleService.clearFaults()
					uni.showToast({ title: '已清除故障', icon: 'success' })
				} catch (err) {
					uni.showToast({ title: '操作失败', icon: 'error' })
				}
			},

			disconnect() {
				bleService.disconnect()
				this.connected = false
				this.statusText = '已断开'
				this.devices = []
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

	/* 标签切换栏 */
	.tab-bar {
		display: flex;
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 16rpx;
		margin-bottom: 30rpx;
		border: 1px solid #2a2a5e;
		overflow: hidden;
	}

	.tab-item {
		flex: 1;
		display: flex;
		align-items: center;
		justify-content: center;
		padding: 20rpx;
		gap: 10rpx;
		opacity: 0.5;
		transition: all 0.3s;
	}

	.tab-item.active {
		opacity: 1;
		background: rgba(79, 195, 247, 0.1);
		border-bottom: 3px solid #4fc3f7;
	}

	.tab-icon {
		font-size: 32rpx;
	}

	.tab-label {
		font-size: 28rpx;
		font-weight: 500;
	}

	/* 扫描区域 */
	.scan-header {
		display: flex;
		justify-content: space-between;
		align-items: center;
		margin-bottom: 24rpx;
	}

	.scan-title {
		font-size: 32rpx;
		font-weight: 600;
		color: #e0e0e0;
	}

	.scan-btn {
		font-size: 26rpx;
		padding: 12rpx 32rpx;
		background: linear-gradient(135deg, #4fc3f7, #0288d1);
		color: #fff;
		border-radius: 40rpx;
		border: none;
	}

	.scan-btn[disabled] {
		opacity: 0.5;
	}

	.scanning-indicator {
		text-align: center;
		padding: 60rpx 0;
	}

	.scanning-text {
		font-size: 28rpx;
		color: #666;
	}

	.empty-state {
		text-align: center;
		padding: 80rpx 0;
	}

	.empty-icon {
		font-size: 80rpx;
		display: block;
		margin-bottom: 20rpx;
	}

	.empty-text {
		font-size: 26rpx;
		color: #666;
	}

	.device-list {
		display: flex;
		flex-direction: column;
		gap: 16rpx;
	}

	.device-item {
		display: flex;
		align-items: center;
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 16rpx;
		padding: 24rpx;
		border: 1px solid #2a2a5e;
	}

	.device-icon {
		font-size: 40rpx;
		margin-right: 20rpx;
	}

	.device-info {
		flex: 1;
	}

	.device-name {
		font-size: 28rpx;
		font-weight: 600;
		color: #e0e0e0;
		display: block;
	}

	.device-id {
		font-size: 20rpx;
		color: #555;
		display: block;
		margin-top: 4rpx;
	}

	.device-rssi {
		font-size: 20rpx;
		color: #888;
		display: block;
		margin-top: 4rpx;
	}

	.device-connect {
		font-size: 28rpx;
		color: #4fc3f7;
	}

	/* 控制面板 */
	.control-panel {
		display: flex;
		flex-direction: column;
		gap: 30rpx;
	}

	/* 电源开关 */
	.power-section {
		display: flex;
		align-items: center;
		gap: 30rpx;
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 20rpx;
		padding: 30rpx;
		border: 1px solid #2a2a5e;
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
	}

	.power-toggle.on {
		background: rgba(0, 230, 118, 0.15);
		border-color: #00e676;
		box-shadow: 0 0 20rpx rgba(0, 230, 118, 0.3);
	}

	.power-icon {
		font-size: 40rpx;
	}

	.power-label {
		font-size: 18rpx;
		color: #888;
		margin-top: 4rpx;
	}

	.power-toggle.on .power-label {
		color: #00e676;
	}

	.power-status {
		flex: 1;
	}

	.power-status-label {
		font-size: 24rpx;
		color: #888;
		display: block;
		margin-bottom: 8rpx;
	}

	.power-status-value {
		font-size: 36rpx;
		font-weight: 600;
		color: #666;
	}

	.power-status-value.on {
		color: #00e676;
	}

	/* 控制滑块区域 */
	.control-section {
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 20rpx;
		padding: 30rpx;
		border: 1px solid #2a2a5e;
	}

	.control-header {
		display: flex;
		justify-content: space-between;
		align-items: center;
		margin-bottom: 16rpx;
	}

	.control-label {
		font-size: 26rpx;
		color: #888;
	}

	.control-value {
		font-size: 36rpx;
		font-weight: 700;
		color: #4fc3f7;
	}

	.control-value.current {
		color: #ff8a65;
	}

	.slider-range {
		display: flex;
		justify-content: space-between;
		font-size: 20rpx;
		color: #555;
		margin-top: 8rpx;
	}

	/* 预设 */
	.preset-section {
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 20rpx;
		padding: 30rpx;
		border: 1px solid #2a2a5e;
	}

	.preset-title {
		font-size: 26rpx;
		color: #888;
		display: block;
		margin-bottom: 20rpx;
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

	/* 操作按钮 */
	.action-buttons {
		display: flex;
		gap: 20rpx;
	}

	.action-btn {
		flex: 1;
		padding: 24rpx;
		border-radius: 16rpx;
		font-size: 28rpx;
		text-align: center;
		border: none;
	}

	.action-btn.danger {
		background: rgba(255, 82, 82, 0.15);
		color: #ff5252;
		border: 1px solid rgba(255, 82, 82, 0.3);
	}

	.action-btn.secondary {
		background: #2a2a5e;
		color: #888;
		border: 1px solid #3a3a6e;
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
</style>
