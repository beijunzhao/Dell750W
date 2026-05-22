<template>
	<view class="container">

		<!-- 主仪表盘 -->
		<view class="dashboard">
			<!-- 输出电压 - 大数字显示 -->
			<view class="meter-card voltage-card" @tap="showVoltageInput">
				<view class="meter-label">输出电压 <text class="tap-hint">点击设置</text></view>
				<view class="meter-value">
					<text class="value-number">{{ powerData.V_out.toFixed(2) }}</text>
					<text class="value-unit">V</text>
				</view>
				<view class="meter-bar">
					<view class="meter-bar-fill voltage-fill" :style="{ width: voltagePercent + '%' }"></view>
				</view>
			</view>
	
			<!-- 输出电流 -->
			<view class="meter-card current-card" @tap="showCurrentInput">
				<view class="meter-label">输出电流 <text class="tap-hint">点击设置</text></view>
				<view class="meter-value">
					<text class="value-number">{{ powerData.I_out.toFixed(2) }}</text>
					<text class="value-unit">A</text>
				</view>
				<view class="meter-bar">
					<view class="meter-bar-fill current-fill" :style="{ width: currentPercent + '%' }"></view>
				</view>
			</view>
		</view>

		<!-- 功率卡片 -->
		<view class="info-grid">
			<view class="info-card power-card">
				<text class="info-label">输出功率</text>
				<text class="info-value">{{ powerData.W_out.toFixed(1) }} <text class="info-unit">W</text></text>
			</view>
			<view class="info-card power-card">
				<text class="info-label">输入功率</text>
				<text class="info-value">{{ powerData.W_in.toFixed(1) }} <text class="info-unit">W</text></text>
			</view>
		</view>

		<!-- 电能累计 -->
		<view class="info-grid">
			<view class="info-card">
				<text class="info-label">累计输出电能</text>
				<text class="info-value">{{ powerData.E_out.toFixed(1) }} <text class="info-unit">Wh</text></text>
			</view>
			<view class="info-card">
				<text class="info-label">累计输入电能</text>
				<text class="info-value">{{ powerData.E_in.toFixed(1) }} <text class="info-unit">Wh</text></text>
			</view>
		</view>

		<!-- 输入参数 -->
		<view class="section-title">输入参数</view>
		<view class="info-grid">
			<view class="info-card">
				<text class="info-label">输入电压</text>
				<text class="info-value">{{ powerData.V_in.toFixed(1) }} <text class="info-unit">V</text></text>
			</view>
			<view class="info-card">
				<text class="info-label">输入电流</text>
				<text class="info-value">{{ powerData.I_in.toFixed(2) }} <text class="info-unit">A</text></text>
			</view>
		</view>

		<!-- 温度 -->
		<view class="section-title">温度</view>
		<view class="info-grid three-col">
			<view class="info-card temp-card" v-for="(temp, idx) in temperatures" :key="idx">
				<text class="info-label">温度 {{ idx + 1 }}</text>
				<text class="info-value temp-value" :class="{ hot: temp > 60 }">
					{{ temp.toFixed(1) }} <text class="info-unit">°C</text>
				</text>
			</view>
		</view>

		<!-- 风扇转速 -->
		<view class="section-title">风扇</view>
		<view class="info-grid">
			<view class="info-card" v-for="(fan, idx) in fanSpeeds" :key="idx">
				<text class="info-label">风扇 {{ idx + 1 }}</text>
				<text class="info-value">{{ fan.toFixed(0) }} <text class="info-unit">RPM</text></text>
			</view>
		</view>

		<!-- 状态 -->
		<view class="section-title">状态</view>
		<view class="status-grid">
			<view class="status-chip" :class="{ ok: powerData.power_on }">
				{{ powerData.power_on ? '运行中' : '已关闭' }}
			</view>
			<view class="status-chip" :class="{ ok: powerData.device_online }">
				{{ powerData.device_online ? 'PMBus 在线' : 'PMBus 离线' }}
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
				</view>
				<view class="modal-footer">
					<view class="modal-btn modal-btn-cancel" @tap="closeModal">取消</view>
					<view class="modal-btn modal-btn-confirm" :class="modalType" @tap="confirmModal">确认设置</view>
				</view>
			</view>
		</view>

		<!-- 底部导航 -->
		<view class="bottom-nav">
			<view class="nav-item active" @tap="switchTab('index')">
				<text class="nav-icon">📊</text>
				<text class="nav-label">监控</text>
			</view>
			<view class="nav-item" @tap="switchTab('control')">
				<text class="nav-icon">🎛️</text>
				<text class="nav-label">控制</text>
			</view>
			<view class="nav-item" @tap="switchTab('settings')">
				<text class="nav-icon">ℹ️</text>
				<text class="nav-label">信息</text>
			</view>
			<!-- 蓝牙状态 -->
			<view class="nav-ble" :class="{ connected: connected }" @tap="handleStatusTap">
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
				statusText: '点击连接设备',
				powerData: {
					V_out: 0, I_out: 0, V_in: 0, I_in: 0,
					W_out: 0, W_in: 0, E_out: 0, E_in: 0,
					temperature: [0, 0, 0],
					fan_speed: [0, 0],
					power_on: 0,
					device_online: false
				},
				// 电源规格 (来自 pin_map.h)
				voltageMax: 12.0,
				currentMax: 60.0,
				// 自定义弹窗状态
				showModal: false,
				modalType: 'voltage', // 'voltage' | 'current'
				modalInputValue: ''
			}
		},
		onLoad() {
			// 全局只初始化一次蓝牙
			bleService.init()

			bleService.onStatus((status) => {
				this.statusText = status
				this.connected = bleService.connected
			})

			bleService.onData((data) => {
				if (data.V_out !== undefined) {
					this.powerData = { ...this.powerData, ...data }
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

		},
		onShow() {
			// 每次页面显示时同步连接状态（连接状态变化由 ble-service.js 全局监听处理）
			this.connected = bleService.connected
			this.statusText = bleService.connected ? '已连接' : '点击连接设备'
		},
		onUnload() {
			this._stopPolling()
			if (this._setTimeout) {
				clearTimeout(this._setTimeout)
				this._setTimeout = null
			}
		},
		computed: {
			temperatures() {
				return this.powerData.temperature || [0, 0, 0]
			},
			fanSpeeds() {
				return this.powerData.fan_speed || [0, 0]
			},
			voltagePercent() {
				return Math.min((this.powerData.V_out / this.voltageMax) * 100, 100)
			},
			currentPercent() {
				return Math.min((this.powerData.I_out / this.currentMax) * 100, 100)
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
					return this.powerData.V_out.toFixed(2)
				}
				return this.powerData.I_out.toFixed(2)
			}
		},
		methods: {

			handleStatusTap() {
				// 跳转到控制页
				uni.navigateTo({
					url: '/pages/control/control'
				})
			},

			switchTab(tab) {
				const urls = {
					index: '/pages/index/index',
					control: '/pages/control/control',
					settings: '/pages/settings/settings'
				}
				// 使用 reLaunch 切换页面 (页面不是 tabBar)
				uni.reLaunch({
					url: urls[tab]
				})
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

				// 检查 BLE 连接参数是否完整
				const deviceId = bleService._deviceId
				const serviceId = bleService._serviceId
				const rxCharId = bleService._rxCharId
				console.log('[监控] BLE参数:', { deviceId, serviceId, rxCharId })

				if (!deviceId || !serviceId || !rxCharId) {
					this._pendingSet = null
					uni.hideLoading()
					uni.showToast({ title: 'BLE参数不完整，请重新连接', icon: 'none' })
					return
				}

				// 通过 bleService.send 发送（它内部做了 JSON.stringify）
				bleService.send({ cmd: 'set', [cmd]: val }).then(() => {
					console.log('[监控] 设置命令已发送')
					uni.showToast({ title: '命令已发送，等待确认...', icon: 'none' })
				}).catch((err) => {
					console.error('[监控] 设置命令发送失败:', err)
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

			_stopPolling() {
				// 页面卸载时不做特殊处理，由 bleService 管理
			}
		}
	}
</script>

<style>
	/* 全局样式 */
	page {
		background-color: #0f0f23;
		color: #e0e0e0;
		font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
	}

	.container {
		padding: 20rpx 30rpx 120rpx;
		min-height: 100vh;
	}

	/* 仪表盘卡片 */
	.dashboard {
		display: flex;
		gap: 20rpx;
		margin-bottom: 24rpx;
	}

	.meter-card {
		flex: 1;
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 20rpx;
		padding: 30rpx;
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

	/* 信息网格 */
	.section-title {
		font-size: 26rpx;
		color: #666;
		margin: 20rpx 0 16rpx;
		padding-left: 8rpx;
		border-left: 6rpx solid #4fc3f7;
		padding-left: 16rpx;
	}

	.info-grid {
		display: flex;
		gap: 20rpx;
		margin-bottom: 8rpx;
	}

	.info-grid.three-col {
		display: flex;
		flex-wrap: wrap;
	}

	.info-grid.three-col .info-card {
		flex: 1;
		min-width: calc(33.33% - 14rpx);
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

	.temp-value.hot {
		color: #ff5252;
	}

	/* 状态标签 */
	.status-grid {
		display: flex;
		flex-wrap: wrap;
		gap: 16rpx;
		margin-bottom: 24rpx;
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

	.status-chip.warn {
		background-color: rgba(255, 82, 82, 0.1);
		color: #ff5252;
		border-color: rgba(255, 82, 82, 0.3);
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
		align-items: center;
		justify-content: center;
		z-index: 999;
		backdrop-filter: blur(4px);
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
</style>
