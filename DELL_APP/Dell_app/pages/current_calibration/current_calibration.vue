<template>
	<view class="container">

		<!-- 标题栏 -->
		<view class="title-bar">
			<text class="title-text">电流参数校准 (Step {{ calStep }}/6)</text>
		</view>

		<!-- 数据面板 -->
		<view class="data-panel">
			<view class="data-row">
				<text class="data-label">校准目标</text>
				<text class="data-value target-color">{{ calTarget.toFixed(1) }} A</text>
			</view>
			<view class="data-row">
				<text class="data-label">实时电流</text>
				<text class="data-value current-color">{{ calCurrent.toFixed(3) }} A</text>
			</view>
			<view class="data-row">
				<text class="data-label">当前 PWM</text>
				<text class="data-value pwm-color">{{ calPwm }}</text>
			</view>
		</view>

		<!-- 安全提示 (降压保护) -->
		<view class="safety-hint">
			<text class="safety-icon">⚠️</text>
			<text class="safety-text">为控制测试功率，系统已自动将输出电压降至 2.0V。请确保已先完成电压校准。</text>
		</view>

		<!-- 操作提示 -->
		<view class="hint-box">
			<text class="hint-icon">💡</text>
			<text class="hint-text">用万用表测量输出端电流，按 UP/DOWN 调节 PWM 使电流等于目标值，然后按 OK 确认</text>
		</view>

		<!-- 步骤进度指示器 -->
		<view class="progress-section">
			<text class="progress-label">校准进度</text>
			<view class="progress-dots">
				<view v-for="i in 6" :key="i" class="progress-dot" :class="{ done: i < calStep, current: i === calStep }">
					<text class="dot-number">{{ i }}</text>
				</view>
			</view>
			<view class="progress-targets">
				<text v-for="(t, i) in targetCurrents" :key="i" class="target-label" :class="{ active: (i + 1) === calStep }">
					{{ t.toFixed(0) }}A
				</text>
			</view>
		</view>

		<!-- 校准结果记录 -->
		<view class="section" v-if="calResults.length > 0">
			<view class="section-header">
				<text class="section-title">校准记录</text>
			</view>
			<view class="result-table">
				<view class="result-header">
					<text class="result-th">Step</text>
					<text class="result-th">目标</text>
					<text class="result-th">原始电流</text>
					<text class="result-th">PWM</text>
				</view>
				<view v-for="(r, i) in calResults" :key="i" class="result-row">
					<text class="result-td">{{ i + 1 }}</text>
					<text class="result-td">{{ r.target.toFixed(1) }}A</text>
					<text class="result-td">{{ r.raw.toFixed(3) }}A</text>
					<text class="result-td">{{ r.pwm }}</text>
				</view>
			</view>
		</view>

		<!-- 控制按钮 -->
		<view class="action-section">
			<!-- 未进入校准模式 -->
			<button v-if="!calMode" class="action-btn enter-btn" :disabled="!connected" @tap="showRangeDialog = true">
				进入校准模式
			</button>

			<!-- 校准模式中的操作按钮 -->
			<view v-else class="cal-controls">
				<!-- 完成状态 -->
				<view v-if="calStep > 6" class="complete-info">
					<text class="complete-text">✅ 校准完成！数据已保存</text>
					<button class="action-btn exit-btn" @tap="exitCalibration">退出校准</button>
				</view>

				<!-- 调节按钮 (按住连续发送) -->
				<view v-else class="adjust-controls">
					<view class="pwm-buttons">
						<button class="pwm-btn pwm-up"
							@touchstart="pwmStart(1)"
							@touchend="pwmStop"
							@touchcancel="pwmStop">
							<text class="pwm-btn-icon">▲</text>
							<text class="pwm-btn-label">增加 PWM</text>
						</button>
						<button class="pwm-btn pwm-down"
							@touchstart="pwmStart(-1)"
							@touchend="pwmStop"
							@touchcancel="pwmStop">
							<text class="pwm-btn-icon">▼</text>
							<text class="pwm-btn-label">减少 PWM</text>
						</button>
					</view>
					<button class="action-btn confirm-btn" @tap="confirmStep">
						✅ 确认 (Step {{ calStep }}/6)
					</button>
					<button class="action-btn exit-small-btn" @tap="exitCalibration">退出校准</button>
				</view>
			</view>
		</view>

		<!-- ===== 量程设置弹窗 (前置步骤) ===== -->
		<view class="modal-overlay" v-if="showRangeDialog" @tap="showRangeDialog = false">
			<view class="modal-box" @tap.stop>
				<view class="modal-header">
					<text class="modal-title">电流量程设置</text>
					<text class="modal-subtitle">请设置当前电源的最大输出电流，系统将自动生成 6 个校准点</text>
				</view>
				<view class="modal-body">
					<view class="input-group">
						<text class="input-label">最大输出电流 (I_MAX)</text>
						<view class="input-wrap">
							<input class="input-field" type="digit" v-model="rangeIMax" placeholder="例如 65.0" />
							<text class="input-unit">A</text>
						</view>
					</view>
					<!-- 预览生成的校准点 -->
					<view class="preview-section" v-if="previewPoints.length > 0">
						<text class="preview-label">即将校准的 6 个电流点:</text>
						<view class="preview-list">
							<text v-for="(p, i) in previewPoints" :key="i" class="preview-item">
								{{ (i + 1) }}. {{ p.toFixed(1) }}A
							</text>
						</view>
					</view>
				</view>
				<view class="modal-footer">
					<button class="modal-btn modal-btn-cancel" @tap="showRangeDialog = false">取消</button>
					<button class="modal-btn modal-btn-confirm" :disabled="!rangeValid" @tap="confirmRange">确认并开始校准</button>
				</view>
			</view>
		</view>

		<!-- 底部导航 -->
		<view class="bottom-nav">
			<view class="nav-item" @tap="switchTab('index')">
				<text class="nav-icon">🏠</text>
				<text class="nav-label">首页</text>
			</view>
			<view class="nav-item" @tap="switchTab('control')">
				<text class="nav-icon">📊</text>
				<text class="nav-label">监控</text>
			</view>
			<view class="nav-item active" @tap="switchTab('current_calibration')">
				<text class="nav-icon">🔧</text>
				<text class="nav-label">电流校准</text>
			</view>
			<view class="nav-ble" :class="{ connected: connected }" @tap="disconnect">
				<view class="nav-ble-dot" :class="{ active: connected }"></view>
			</view>
		</view>
	</view>
</template>

<script>
	import bleService from '../../utils/ble-service.js'

	// 按住连续发送间隔 (ms)
	const PWM_HOLD_INTERVAL = 100

	/**
	 * 根据量程生成 6 个平滑分布的校准目标点
	 * 算法: 第 1 点为 0.0, 最后 1 点为 maxVal, 中间 4 点均分
	 * @param {number} maxVal - 最大量程值
	 * @returns {number[]} 6 个校准点数组
	 */
	function generateCalPoints(maxVal) {
		if (maxVal <= 0) return [0, 0, 0, 0, 0, 0]
		const points = [0.0]
		for (let i = 1; i < 5; i++) {
			points.push(parseFloat((maxVal * i / 5).toFixed(3)))
		}
		points.push(parseFloat(maxVal.toFixed(3)))
		return points
	}

	export default {
		data() {
			return {
				connected: false,
				// 量程设置
				showRangeDialog: false,
				rangeIMax: '',
				// 动态生成的校准点
				targetCurrents: [0.0, 5.0, 15.0, 30.0, 45.0, 60.0],
				calMode: false,
				calStep: 1,
				calTarget: 0.0,
				calCurrent: 0,
				calPwm: 0,
				calResults: [],
				_pwmTimer: null,
				_pwmDir: 0,
				/** 进入页面前的电压值，用于退出时恢复 */
				_prevVSet: 0,
				/** 安全降压目标 */
				SAFE_VOLTAGE: 2.0
			}
		},
		computed: {
			/** 量程输入是否有效 */
			rangeValid() {
				const i = parseFloat(this.rangeIMax)
				return i > 0
			},
			/** 预览即将生成的 6 个电流校准点 */
			previewPoints() {
				const i = parseFloat(this.rangeIMax)
				if (!(i > 0)) return []
				return generateCalPoints(i)
			}
		},
		onLoad() {
			bleService.onStatus((status) => {
				this.connected = bleService.connected
			})
			this.connected = bleService.connected
		},
		onShow() {
			this.connected = bleService.connected

			// ===== 安全保护: 进入电流校准页面时自动降压至 2.0V =====
			if (this.connected) {
				// 保存当前电压值用于退出时恢复
				const lastData = bleService.getLastData()
				if (lastData && lastData.V_set !== undefined) {
					this._prevVSet = lastData.V_set
				}
				// 发送降压指令 (fire-and-forget, 不阻塞 UI)
				bleService.setVoltage(this.SAFE_VOLTAGE).catch(err => {
					console.error('[ICal] 自动降压失败:', err)
				})
			}

			// 注册数据回调，实时更新校准数据
			bleService.onData((data) => {
				if (data.cal_mode !== undefined) {
					this.calMode = data.cal_mode === 1 || data.cal_mode === true
				}
				if (data.cal_step !== undefined) {
					this.calStep = data.cal_step
					this.calTarget = this.targetCurrents[this.calStep - 1] || 0
				}
				if (data.cal_target !== undefined) {
					this.calTarget = data.cal_target
				}
				if (data.I_out !== undefined) this.calCurrent = data.I_out
				if (data.cal_pwm !== undefined) this.calPwm = data.cal_pwm
				// 校准结果记录 (ESP32 推送的 cal_result_* 数据)
				if (data.cal_result_target !== undefined) {
					this.calResults.push({
						target: data.cal_result_target,
						raw: data.cal_result_adc || 0,
						pwm: data.cal_result_pwm || 0
					})
				}
			})
			// 从缓存加载初始值
			const data = bleService.getLastData()
			if (data) {
				if (data.cal_mode !== undefined) this.calMode = data.cal_mode === 1 || data.cal_mode === true
				if (data.cal_step !== undefined) {
					this.calStep = data.cal_step
					this.calTarget = this.targetCurrents[this.calStep - 1] || 0
				}
				if (data.cal_target !== undefined) this.calTarget = data.cal_target
				if (data.I_out !== undefined) this.calCurrent = data.I_out
				if (data.cal_pwm !== undefined) this.calPwm = data.cal_pwm
			}
		},
		// 离开页面时: 停止 PWM 定时器 + 关闭输出防止意外
		onUnload() {
			this._stopPwmTimer()
			// 退出时关闭输出 (安全保护)
			if (this.connected) {
				bleService.send({ cmd: 'set', power: 0 }).catch(err => {
					console.error('[ICal] 退出关闭输出失败:', err)
				})
			}
		},
		methods: {
			/**
			 * 确认量程设置 → 下发 set_range → 生成校准点 → 进入校准模式
			 */
			async confirmRange() {
				if (!this.connected) return
				const iMax = parseFloat(this.rangeIMax)
				if (!(iMax > 0)) {
					uni.showToast({ title: '请输入有效的最大电流', icon: 'none' })
					return
				}
				uni.showLoading({ title: '设置量程...' })
				try {
					// 1. 下发电流量程到设备
					await bleService.setRange(0, iMax)
					// 2. 动态生成 6 个电流校准点
					this.targetCurrents = generateCalPoints(iMax)
					// 3. 关闭弹窗，进入校准模式
					this.showRangeDialog = false
					await bleService.enterCalMode()
					this.calMode = true
					this.calStep = 1
					this.calTarget = this.targetCurrents[0]
					this.calResults = []
					uni.hideLoading()
					uni.showToast({ title: '已进入校准模式', icon: 'success' })
				} catch (err) {
					uni.hideLoading()
					uni.showToast({ title: '进入失败: ' + (err.message || ''), icon: 'none' })
				}
			},
			/**
			 * 按住按钮开始连续发送 PWM 调节命令
			 * @param {number} dir 1=增加, -1=减少
			 */
			pwmStart(dir) {
				if (!this.connected) return
				this._pwmDir = dir
				// 立即发送一次
				this._sendPwmOnce()
				// 启动定时器连续发送
				this._pwmTimer = setInterval(() => {
					this._sendPwmOnce()
				}, PWM_HOLD_INTERVAL)
			},
			/**
			 * 松开按钮停止发送
			 */
			pwmStop() {
				this._stopPwmTimer()
				this._pwmDir = 0
			},
			_stopPwmTimer() {
				if (this._pwmTimer) {
					clearInterval(this._pwmTimer)
					this._pwmTimer = null
				}
			},
			/**
			 * 发送单次 PWM 调节命令 (fire-and-forget, 不等待)
			 */
			_sendPwmOnce() {
				if (!this.connected) return
				if (this._pwmDir > 0) {
					bleService.calPwmUp().catch(err => {
						console.error('PWM UP failed:', err)
					})
				} else if (this._pwmDir < 0) {
					bleService.calPwmDown().catch(err => {
						console.error('PWM DOWN failed:', err)
					})
				}
			},
			async exitCalibration() {
				try {
					await bleService.exitCalMode()
					this.calMode = false
					this.calStep = 1
					uni.showToast({ title: '已退出校准', icon: 'success' })
				} catch (err) {
					uni.showToast({ title: '退出失败', icon: 'error' })
				}
			},
			async confirmStep() {
				if (!this.connected) return
				try {
					await bleService.calConfirm()
					uni.showToast({ title: `Step ${this.calStep} 已确认`, icon: 'success' })
				} catch (err) {
					uni.showToast({ title: '确认失败', icon: 'error' })
				}
			},
			/**
			 * 所有 6 点完成后提交电流校准表到设备
			 */
			async submitCalTable() {
				if (!this.connected || this.calResults.length < 6) return
				uni.showLoading({ title: '保存校准表...' })
				try {
					// 构建 6 点校准表数据
					const points = this.calResults.map((r, i) => ({
						r: r.raw,       // PMBus 原始电流
						v: r.target,    // 目标电流
						p: r.pwm        // PWM 值
					}))
					await bleService.setICalTable(points)
					uni.hideLoading()
					uni.showToast({ title: '电流校准表已保存', icon: 'success' })
				} catch (err) {
					uni.hideLoading()
					uni.showToast({ title: '保存失败: ' + (err.message || ''), icon: 'none' })
				}
			},
			disconnect() {
				bleService.disconnect()
				this.connected = false
			},
			switchTab(tab) {
				const urls = {
					index: '/pages/index/index',
					control: '/pages/control/control',
					current_calibration: '/pages/current_calibration/current_calibration'
				}
				uni.reLaunch({ url: urls[tab] })
			}
		},
		// 监听 calResults 变化: 当 6 点全部完成时自动提交
		watch: {
			calResults: {
				handler(newVal) {
					if (newVal.length >= 6 && this.calMode && this.calStep > 6) {
						// 延迟一下等待 ESP32 完成状态切换
						setTimeout(() => {
							this.submitCalTable()
						}, 500)
					}
				},
				deep: true
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
		padding: 0 30rpx 120rpx;
		min-height: 100vh;
	}

	/* 标题栏 */
	.title-bar {
		background: linear-gradient(135deg, #00695c, #00897b);
		margin: 0 -30rpx;
		padding: 28rpx 30rpx;
	}

	.title-text {
		font-size: 32rpx;
		font-weight: 700;
		color: #fff;
	}

	/* 数据面板 */
	.data-panel {
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 20rpx;
		padding: 30rpx;
		margin-top: 24rpx;
		border: 1px solid #2a2a5e;
	}

	.data-row {
		display: flex;
		justify-content: space-between;
		align-items: center;
		padding: 16rpx 0;
		border-bottom: 1px solid #1a1a3e;
	}

	.data-row:last-child {
		border-bottom: none;
	}

	.data-label {
		font-size: 28rpx;
		color: #888;
	}

	.data-value {
		font-size: 36rpx;
		font-weight: 700;
		font-variant-numeric: tabular-nums;
	}

	.target-color {
		color: #ffd54f;
	}

	.current-color {
		color: #66bb6a;
	}

	.pwm-color {
		color: #4fc3f7;
	}

	/* 安全提示 (降压保护) */
	.safety-hint {
		background: #2a1f00;
		border-radius: 16rpx;
		padding: 20rpx 24rpx;
		margin-top: 24rpx;
		border-left: 6rpx solid #ff8f00;
		display: flex;
		align-items: flex-start;
	}

	.safety-icon {
		font-size: 32rpx;
		margin-right: 14rpx;
		flex-shrink: 0;
	}

	.safety-text {
		font-size: 24rpx;
		color: #ffb74d;
		line-height: 1.6;
		flex: 1;
	}

	/* 提示框 */
	.hint-box {
		background: #1a2740;
		border-radius: 16rpx;
		padding: 24rpx;
		margin-top: 24rpx;
		border-left: 6rpx solid #4fc3f7;
		display: flex;
		align-items: flex-start;
	}

	.hint-icon {
		font-size: 36rpx;
		margin-right: 16rpx;
	}

	.hint-text {
		font-size: 26rpx;
		color: #4fc3f7;
		line-height: 1.6;
		flex: 1;
	}

	/* 进度指示器 */
	.progress-section {
		margin-top: 30rpx;
		text-align: center;
	}

	.progress-label {
		font-size: 24rpx;
		color: #888;
		margin-bottom: 16rpx;
		display: block;
	}

	.progress-dots {
		display: flex;
		justify-content: center;
		gap: 20rpx;
		margin-bottom: 10rpx;
	}

	.progress-dot {
		width: 56rpx;
		height: 56rpx;
		border-radius: 50%;
		background: #1a1a3e;
		border: 3rpx solid #2a2a5e;
		display: flex;
		align-items: center;
		justify-content: center;
		transition: all 0.3s;
	}

	.progress-dot.done {
		background: #2e7d32;
		border-color: #4caf50;
	}

	.progress-dot.current {
		background: #00897b;
		border-color: #4db6ac;
		transform: scale(1.2);
	}

	.dot-number {
		font-size: 24rpx;
		font-weight: 700;
		color: #e0e0e0;
	}

	.progress-dot.current .dot-number {
		color: #fff;
	}

	.progress-targets {
		display: flex;
		justify-content: center;
		gap: 20rpx;
		margin-top: 8rpx;
	}

	.target-label {
		width: 56rpx;
		font-size: 20rpx;
		color: #666;
		text-align: center;
	}

	.target-label.active {
		color: #4db6ac;
		font-weight: 600;
	}

	/* 校准记录 */
	.section {
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 20rpx;
		padding: 30rpx;
		margin-top: 24rpx;
		border: 1px solid #2a2a5e;
	}

	.section-header {
		margin-bottom: 20rpx;
		padding-bottom: 16rpx;
		border-bottom: 1px solid #2a2a5e;
	}

	.section-title {
		font-size: 28rpx;
		font-weight: 600;
		color: #e0e0e0;
	}

	.result-table {
		display: flex;
		flex-direction: column;
	}

	.result-header {
		display: flex;
		padding: 8rpx 0;
		border-bottom: 1px solid #2a2a5e;
	}

	.result-th {
		flex: 1;
		font-size: 22rpx;
		color: #888;
		text-align: center;
	}

	.result-row {
		display: flex;
		padding: 12rpx 0;
		border-bottom: 1px solid #1a1a3e;
	}

	.result-row:last-child {
		border-bottom: none;
	}

	.result-td {
		flex: 1;
		font-size: 24rpx;
		color: #e0e0e0;
		text-align: center;
	}

	/* 控制按钮 */
	.action-section {
		margin-top: 30rpx;
	}

	.action-btn {
		width: 100%;
		padding: 28rpx;
		border-radius: 16rpx;
		font-size: 30rpx;
		border: none;
		color: #fff;
	}

	.enter-btn {
		background: linear-gradient(135deg, #00897b, #004d40);
	}

	.exit-btn {
		background: linear-gradient(135deg, #ef5350, #c62828);
	}

	.action-btn[disabled] {
		opacity: 0.4;
	}

	/* 校准控制区 */
	.cal-controls {
		display: flex;
		flex-direction: column;
		gap: 20rpx;
	}

	.adjust-controls {
		display: flex;
		flex-direction: column;
		gap: 20rpx;
	}

	.pwm-buttons {
		display: flex;
		gap: 20rpx;
	}

	.pwm-btn {
		flex: 1;
		padding: 24rpx;
		border-radius: 16rpx;
		border: none;
		color: #fff;
		display: flex;
		flex-direction: column;
		align-items: center;
		gap: 8rpx;
	}

	.pwm-up {
		background: linear-gradient(135deg, #2e7d32, #4caf50);
	}

	.pwm-up:active {
		background: linear-gradient(135deg, #1b5e20, #388e3c);
		transform: scale(0.95);
	}

	.pwm-down {
		background: linear-gradient(135deg, #c62828, #ef5350);
	}

	.pwm-down:active {
		background: linear-gradient(135deg, #b71c1c, #d32f2f);
		transform: scale(0.95);
	}

	.pwm-btn-icon {
		font-size: 40rpx;
	}

	.pwm-btn-label {
		font-size: 24rpx;
	}

	.confirm-btn {
		background: linear-gradient(135deg, #1565c0, #42a5f5);
	}

	.exit-small-btn {
		background: transparent;
		border: 2rpx solid #666;
		color: #888;
		padding: 16rpx;
		font-size: 26rpx;
	}

	.complete-info {
		display: flex;
		flex-direction: column;
		align-items: center;
		gap: 24rpx;
	}

	.complete-text {
		font-size: 32rpx;
		color: #4caf50;
		font-weight: 600;
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
		color: #00897b;
	}

	.nav-icon {
		font-size: 44rpx;
		margin-bottom: 4rpx;
	}

	.nav-label {
		font-size: 22rpx;
		color: #666;
	}

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

	/* ===== 量程设置弹窗 ===== */
	.modal-overlay {
		position: fixed;
		top: 0;
		left: 0;
		right: 0;
		bottom: 0;
		background: rgba(0, 0, 0, 0.7);
		display: flex;
		align-items: center;
		justify-content: center;
		z-index: 999;
	}

	.modal-box {
		background: #1a1a3e;
		border-radius: 24rpx;
		width: 86%;
		max-width: 600rpx;
		border: 1px solid #2a2a5e;
		overflow: hidden;
	}

	.modal-header {
		padding: 32rpx 30rpx 20rpx;
		text-align: center;
	}

	.modal-title {
		font-size: 34rpx;
		font-weight: 700;
		color: #e0e0e0;
		display: block;
		margin-bottom: 12rpx;
	}

	.modal-subtitle {
		font-size: 24rpx;
		color: #888;
		line-height: 1.6;
		display: block;
	}

	.modal-body {
		padding: 0 30rpx 20rpx;
	}

	.input-group {
		margin-bottom: 24rpx;
	}

	.input-label {
		font-size: 26rpx;
		color: #aaa;
		display: block;
		margin-bottom: 10rpx;
	}

	.input-wrap {
		display: flex;
		align-items: center;
		background: #0f0f23;
		border: 1px solid #2a2a5e;
		border-radius: 12rpx;
		padding: 0 20rpx;
	}

	.input-field {
		flex: 1;
		height: 80rpx;
		font-size: 30rpx;
		color: #e0e0e0;
		background: transparent;
		border: none;
		outline: none;
	}

	.input-unit {
		font-size: 28rpx;
		color: #888;
		margin-left: 10rpx;
	}

	.preview-section {
		background: #0f0f23;
		border-radius: 12rpx;
		padding: 16rpx 20rpx;
		margin-top: 8rpx;
	}

	.preview-label {
		font-size: 24rpx;
		color: #4db6ac;
		display: block;
		margin-bottom: 10rpx;
	}

	.preview-list {
		display: flex;
		flex-wrap: wrap;
		gap: 8rpx;
	}

	.preview-item {
		font-size: 22rpx;
		color: #aaa;
		background: #1a1a3e;
		padding: 6rpx 14rpx;
		border-radius: 8rpx;
	}

	.modal-footer {
		display: flex;
		border-top: 1px solid #2a2a5e;
	}

	.modal-btn {
		flex: 1;
		height: 88rpx;
		line-height: 88rpx;
		text-align: center;
		font-size: 30rpx;
		border: none;
		background: transparent;
		color: #e0e0e0;
	}

	.modal-btn-cancel {
		color: #888;
		border-right: 1px solid #2a2a5e;
	}

	.modal-btn-confirm {
		color: #4db6ac;
		font-weight: 600;
	}

	.modal-btn-confirm[disabled] {
		opacity: 0.4;
	}
</style>
