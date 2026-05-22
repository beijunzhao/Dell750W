<template>
	<view class="container">

		<!-- 未连接提示 -->
		<view v-if="!connected" class="disconnected-hint">
			<text class="disconnected-icon">📡</text>
			<text class="disconnected-text">未连接设备</text>
			<text class="disconnected-sub">请前往首页点击蓝牙按钮连接设备</text>
			<button class="goto-home-btn" @tap="switchTab('index')">前往首页</button>
		</view>

		<!-- 监控面板 (已连接时显示) -->
		<view v-if="connected" class="monitor-panel">

			<!-- 主仪表盘 - 输入电压/输入电流 -->
			<view class="dashboard">
				<view class="meter-card voltage-card">
					<view class="meter-label">输入电压</view>
					<view class="meter-value">
						<text class="value-number">{{ powerData.V_in.toFixed(1) }}</text>
						<text class="value-unit">V</text>
					</view>
					<view class="meter-bar">
						<view class="meter-bar-fill voltage-fill" :style="{ width: voltagePercent + '%' }"></view>
					</view>
				</view>

				<view class="meter-card current-card">
					<view class="meter-label">输入电流</view>
					<view class="meter-value">
						<text class="value-number">{{ powerData.I_in.toFixed(2) }}</text>
						<text class="value-unit">A</text>
					</view>
					<view class="meter-bar">
						<view class="meter-bar-fill current-fill" :style="{ width: currentPercent + '%' }"></view>
					</view>
				</view>
			</view>

			<!-- 功率 & 电能合并大卡片 -->
			<view class="power-energy-card">
				<view class="pe-item">
					<text class="pe-label">输入功率</text>
					<text class="pe-value">{{ powerData.W_in.toFixed(1) }} <text class="pe-unit">W</text></text>
				</view>
				<view class="pe-divider"></view>
				<view class="pe-item">
					<text class="pe-label">累计输入电能</text>
					<text class="pe-value">{{ powerData.E_in.toFixed(1) }} <text class="pe-unit">Wh</text></text>
				</view>
			</view>

			<!-- 温度 & 风扇区域 -->
			<view class="env-row">
				<!-- 温度卡片 -->
				<view class="env-card temp-card-bg">
					<view class="env-card-header">
						<text class="env-card-icon">🌡️</text>
						<text class="env-card-title">温度</text>
					</view>
					<view class="temp-ring-container">
						<view
							v-for="(temp, idx) in temperatures"
							:key="idx"
							class="temp-ring-item"
						>
							<view class="temp-ring">
								<svg viewBox="0 0 36 36" class="temp-ring-svg">
									<path class="temp-ring-bg"
										d="M18 2.0845 a 15.9155 15.9155 0 0 1 0 31.831 a 15.9155 15.9155 0 0 1 0 -31.831"
									/>
									<path class="temp-ring-fill"
										:class="tempRingClass(temp)"
										:stroke-dasharray="tempRingDasharray(temp)"
										d="M18 2.0845 a 15.9155 15.9155 0 0 1 0 31.831 a 15.9155 15.9155 0 0 1 0 -31.831"
									/>
								</svg>
								<view class="temp-ring-text">
									<text class="temp-ring-value" :class="{ hot: temp > 60 }">{{ temp.toFixed(1) }}</text>
									<text class="temp-ring-unit">°C</text>
								</view>
							</view>
							<text class="temp-ring-label">T{{ idx + 1 }}</text>
						</view>
					</view>
				</view>

				<!-- 风扇卡片 -->
				<view class="env-card fan-card-bg">
					<view class="env-card-header">
						<text class="env-card-icon">🌀</text>
						<text class="env-card-title">风扇</text>
					</view>
					<view class="fan-body">
						<view class="fan-blade-container" :class="{ spinning: fanSpeed > 0 }">
							<view class="fan-blade fan-blade-1"></view>
							<view class="fan-blade fan-blade-2"></view>
							<view class="fan-blade fan-blade-3"></view>
							<view class="fan-hub"></view>
						</view>
						<view class="fan-info">
							<text class="fan-speed">{{ fanSpeed.toFixed(0) }}</text>
							<text class="fan-unit">RPM</text>
						</view>
					</view>
				</view>
			</view>

				<!-- 转换效率（全宽） -->
				<view class="efficiency-full-card">
					<view class="env-card-header">
						<text class="env-card-icon">📈</text>
						<text class="env-card-title">转换效率</text>
					</view>
					<view class="efficiency-body">
						<view class="efficiency-big-ring">
							<svg viewBox="0 0 36 36" class="efficiency-ring-svg">
								<path class="temp-ring-bg"
									d="M18 2.0845 a 15.9155 15.9155 0 0 1 0 31.831 a 15.9155 15.9155 0 0 1 0 -31.831"
								/>
								<path class="efficiency-ring-fill"
									:stroke-dasharray="efficiencyDasharray"
									d="M18 2.0845 a 15.9155 15.9155 0 0 1 0 31.831 a 15.9155 15.9155 0 0 1 0 -31.831"
								/>
							</svg>
							<view class="efficiency-ring-text">
								<text class="efficiency-big-value">{{ efficiencyPercent }}</text>
								<text class="efficiency-ring-unit">%</text>
							</view>
						</view>
						<view class="efficiency-bars">
							<view class="efficiency-bar-item">
								<view class="efficiency-bar-label">
									<text>输入功率</text>
									<text class="efficiency-bar-val">{{ powerData.W_in.toFixed(1) }} W</text>
								</view>
								<view class="efficiency-bar-track">
									<view class="efficiency-bar-fill efficiency-bar-in" :style="{ width: efficiencyBarIn + '%' }"></view>
								</view>
							</view>
							<view class="efficiency-bar-item">
								<view class="efficiency-bar-label">
									<text>输出功率</text>
									<text class="efficiency-bar-val">{{ powerData.W_out.toFixed(1) }} W</text>
								</view>
								<view class="efficiency-bar-track">
									<view class="efficiency-bar-fill efficiency-bar-out" :style="{ width: efficiencyBarOut + '%' }"></view>
								</view>
							</view>
						</view>
					</view>
				</view>

				<!-- 实时数据曲线 -->
				<view class="chart-card">
					<view class="env-card-header">
						<text class="env-card-icon">📉</text>
						<text class="env-card-title">实时趋势</text>
					</view>
					<view class="chart-body">
						<!-- 图例 -->
						<view class="chart-legend">
							<view class="chart-legend-item">
								<view class="chart-legend-dot" style="background:#4fc3f7"></view>
								<text class="chart-legend-label">电压</text>
							</view>
							<view class="chart-legend-item">
								<view class="chart-legend-dot" style="background:#ff8a65"></view>
								<text class="chart-legend-label">电流</text>
							</view>
							<view class="chart-legend-item">
								<view class="chart-legend-dot" style="background:#ffd54f"></view>
								<text class="chart-legend-label">功率</text>
							</view>
						</view>
						<!-- SVG 曲线 -->
						<view class="chart-svg-wrap">
							<svg viewBox="0 0 300 60" class="chart-svg" v-if="hasChartData">
								<!-- 网格线 -->
								<line class="chart-gridline" x1="0" y1="15" x2="300" y2="15"/>
								<line class="chart-gridline" x1="0" y1="30" x2="300" y2="30"/>
								<line class="chart-gridline" x1="0" y1="45" x2="300" y2="45"/>
								<!-- 折线 -->
								<path class="chart-line chart-line-vout" :d="chartPathVout"/>
								<path class="chart-line chart-line-iout" :d="chartPathIout"/>
								<path class="chart-line chart-line-wout" :d="chartPathWout"/>
							</svg>
							<view v-else class="chart-empty">
								<text class="chart-empty-text">等待数据...</text>
							</view>
						</view>
					</view>
				</view>
			</view>

			<!-- 底部导航 -->
		<view class="bottom-nav">
			<view class="nav-item" @tap="switchTab('index')">
				<text class="nav-icon">🏠</text>
				<text class="nav-label">首页</text>
			</view>
			<view class="nav-item active" @tap="switchTab('control')">
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
				powerData: {
					V_out: 0, I_out: 0, V_in: 0, I_in: 0,
					W_out: 0, W_in: 0, E_out: 0, E_in: 0,
					temperature: [0, 0, 0],
					fan_speed: 0,
					power_on: 0,
					device_online: false
				},
				voltageMax: 380.0,  // 输入电压范围 ~380V
				currentMax: 5.0,     // 输入电流范围 ~5A
				chartHistory: [],    // 实时曲线历史数据 [{t, V_out, I_out, W_out}]
				chartMaxPoints: 40  // 最多保留40个点
			}
		},
		onLoad() {
			bleService.onStatus((status) => {
				this.connected = bleService.connected
			})

			bleService.onData((data) => {
				if (data.V_out !== undefined) {
					this.powerData = { ...this.powerData, ...data }
					// 推入曲线历史数据
					this.chartHistory.push({
						t: Date.now(),
						V_out: data.V_out || 0,
						I_out: data.I_out || 0,
						W_out: data.W_out || 0
					})
					// 限制长度
					if (this.chartHistory.length > this.chartMaxPoints) {
						this.chartHistory = this.chartHistory.slice(-this.chartMaxPoints)
					}
				}
			})
		},
		onShow() {
			this.connected = bleService.connected
		},
		computed: {
			temperatures() {
				return this.powerData.temperature || [0, 0, 0]
			},
			fanSpeed() {
				const val = this.powerData.fan_speed
				if (Array.isArray(val)) return val[0] || 0
				return val || 0
			},
			voltagePercent() {
				return Math.min((this.powerData.V_in / this.voltageMax) * 100, 100)
			},
			currentPercent() {
				return Math.min((this.powerData.I_in / this.currentMax) * 100, 100)
			},
			tempRingClass() {
				return (temp) => {
					if (temp > 60) return 'hot'
					if (temp > 40) return 'warm'
					return 'cool'
				}
			},
			tempRingDasharray() {
				return (temp) => {
					const maxTemp = 100
					const pct = Math.min(temp / maxTemp, 1)
					const circumference = 2 * Math.PI * 15.9155
					return `${pct * circumference} ${circumference}`
				}
			},
			efficiencyPercent() {
				const win = this.powerData.W_in || 0
				const wout = this.powerData.W_out || 0
				if (win <= 0) return 0
				return Math.min(Math.round((wout / win) * 100), 100)
			},
			efficiencyDasharray() {
				const circumference = 2 * Math.PI * 15.9155
				const pct = this.efficiencyPercent / 100
				return `${pct * circumference} ${circumference}`
			},
			// 实时曲线 - 输出电压 SVG path
			chartPathVout() {
				return this._buildChartPath('V_out', 30, 0, '#4fc3f7')
			},
			// 实时曲线 - 输出电流 SVG path
			chartPathIout() {
				return this._buildChartPath('I_out', 3, 0, '#ff8a65')
			},
			// 实时曲线 - 输出功率 SVG path
			chartPathWout() {
				return this._buildChartPath('W_out', 100, 0, '#ffd54f')
			},
			// 是否有足够数据画图
			hasChartData() {
				return this.chartHistory.length >= 2
			},
			// 效率进度条 - 输入功率占比
			efficiencyBarIn() {
				const win = this.powerData.W_in || 0
				const wout = this.powerData.W_out || 0
				const max = Math.max(win, wout, 1)
				return Math.min((win / max) * 100, 100)
			},
			// 效率进度条 - 输出功率占比
			efficiencyBarOut() {
				const win = this.powerData.W_in || 0
				const wout = this.powerData.W_out || 0
				const max = Math.max(win, wout, 1)
				return Math.min((wout / max) * 100, 100)
			}
		},
		methods: {
			// 构建 SVG 折线 path
			_buildChartPath(key, maxVal, minVal, color) {
				const data = this.chartHistory
				if (data.length < 2) return ''
				const w = 300, h = 60
				const range = maxVal - minVal || 1
				const stepX = w / (data.length - 1)
				const points = data.map((d, i) => {
					const x = i * stepX
					const val = Math.min(Math.max(d[key] || 0, minVal), maxVal)
					const y = h - ((val - minVal) / range) * h
					return `${x},${y}`
				})
				return `M${points.join(' L')}`
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
		padding: 20rpx 28rpx 110rpx;
		min-height: 100vh;
	}

	/* 未连接提示 */
	.disconnected-hint {
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
		padding: 120rpx 0;
	}

	.disconnected-icon {
		font-size: 100rpx;
		margin-bottom: 30rpx;
	}

	.disconnected-text {
		font-size: 36rpx;
		font-weight: 600;
		color: #e0e0e0;
		margin-bottom: 16rpx;
	}

	.disconnected-sub {
		font-size: 26rpx;
		color: #666;
		margin-bottom: 40rpx;
		text-align: center;
	}

	.goto-home-btn {
		padding: 20rpx 60rpx;
		background: linear-gradient(135deg, #4fc3f7, #0288d1);
		color: #fff;
		border-radius: 40rpx;
		font-size: 28rpx;
		border: none;
	}

	/* 监控面板 */
	.monitor-panel {
		display: flex;
		flex-direction: column;
		gap: 10rpx;
	}

	/* 仪表盘卡片 */
	.dashboard {
		display: flex;
		gap: 20rpx;
		margin-bottom: 14rpx;
	}

	.meter-card {
		flex: 1;
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 18rpx;
		padding: 26rpx 28rpx;
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
		margin-bottom: 10rpx;
	}

	.meter-value {
		display: flex;
		align-items: baseline;
		margin-bottom: 14rpx;
	}

	.value-number {
		font-size: 60rpx;
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
		font-size: 26rpx;
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

	/* 功率&电能合并大卡片 */
	.power-energy-card {
		display: flex;
		align-items: center;
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 18rpx;
		padding: 30rpx 28rpx;
		border: 1px solid #ffd54f;
		margin-bottom: 14rpx;
	}

	.pe-item {
		flex: 1;
		text-align: center;
	}

	.pe-label {
		font-size: 24rpx;
		color: #888;
		display: block;
		margin-bottom: 10rpx;
	}

	.pe-value {
		font-size: 48rpx;
		font-weight: 700;
		color: #ffd54f;
	}

	.pe-unit {
		font-size: 26rpx;
		font-weight: 400;
		color: #ffd54f;
		opacity: 0.7;
	}

	.pe-divider {
		width: 1px;
		height: 70rpx;
		background: rgba(255, 213, 79, 0.2);
		margin: 0 20rpx;
	}

	/* 温度 & 风扇 - 双卡片布局 */
	.env-row {
		display: flex;
		gap: 20rpx;
		margin-bottom: 14rpx;
	}

	.env-card {
		flex: 1;
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 18rpx;
		padding: 20rpx 22rpx;
		border: 1px solid #2a2a5e;
	}

	.temp-card-bg {
		border-color: #ff7043;
	}

	.fan-card-bg {
		border-color: #4fc3f7;
	}

	.env-card-header {
		display: flex;
		align-items: center;
		gap: 8rpx;
		margin-bottom: 16rpx;
	}

	.env-card-icon {
		font-size: 28rpx;
	}

	.env-card-title {
		font-size: 24rpx;
		font-weight: 600;
		color: #ccc;
	}

	/* 温度环形进度条 */
	.temp-ring-container {
		display: flex;
		justify-content: space-around;
		gap: 8rpx;
	}

	.temp-ring-item {
		display: flex;
		flex-direction: column;
		align-items: center;
		gap: 8rpx;
	}

	.temp-ring {
		position: relative;
		width: 92rpx;
		height: 92rpx;
		display: flex;
		align-items: center;
		justify-content: center;
	}

	.temp-ring-svg {
		width: 100%;
		height: 100%;
		transform: rotate(-90deg);
	}

	.temp-ring-bg {
		fill: none;
		stroke: #2a2a5e;
		stroke-width: 2.5;
	}

	.temp-ring-fill {
		fill: none;
		stroke-width: 2.5;
		stroke-linecap: round;
		transition: stroke-dasharray 0.6s ease, stroke 0.3s ease;
	}

	.temp-ring-fill.cool {
		stroke: #4fc3f7;
	}

	.temp-ring-fill.warm {
		stroke: #ffd54f;
	}

	.temp-ring-fill.hot {
		stroke: #ff5252;
	}

	.temp-ring-text {
		position: absolute;
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
	}

	.temp-ring-value {
		font-size: 26rpx;
		font-weight: 700;
		color: #e0e0e0;
		line-height: 1.1;
	}

	.temp-ring-value.hot {
		color: #ff5252;
	}

	.temp-ring-unit {
		font-size: 16rpx;
		color: #888;
		line-height: 1;
	}

	.temp-ring-label {
		font-size: 20rpx;
		color: #666;
	}

	/* 风扇动画 */
	.fan-body {
		display: flex;
		flex-direction: column;
		align-items: center;
		gap: 14rpx;
		padding: 6rpx 0;
	}

	.fan-blade-container {
		position: relative;
		width: 110rpx;
		height: 110rpx;
		display: flex;
		align-items: center;
		justify-content: center;
	}

	.fan-blade-container.spinning {
		animation: fanSpin 1.2s linear infinite;
	}

	.fan-blade-container.spinning .fan-blade-1 {
		animation: fanBlade1 1.2s ease-in-out infinite;
	}

	.fan-blade-container.spinning .fan-blade-2 {
		animation: fanBlade2 1.2s ease-in-out infinite;
	}

	.fan-blade-container.spinning .fan-blade-3 {
		animation: fanBlade3 1.2s ease-in-out infinite;
	}

	.fan-blade {
		position: absolute;
		width: 36rpx;
		height: 50rpx;
		background: linear-gradient(180deg, rgba(79, 195, 247, 0.9), rgba(2, 136, 209, 0.6));
		border-radius: 50% 50% 20% 20%;
		transform-origin: center 55rpx;
		opacity: 0.85;
	}

	.fan-blade-1 {
		transform: rotate(0deg) translateY(-28rpx);
	}

	.fan-blade-2 {
		transform: rotate(120deg) translateY(-28rpx);
	}

	.fan-blade-3 {
		transform: rotate(240deg) translateY(-28rpx);
	}

	.fan-hub {
		width: 22rpx;
		height: 22rpx;
		border-radius: 50%;
		background: radial-gradient(circle, #4fc3f7, #0288d1);
		z-index: 2;
		box-shadow: 0 0 12rpx rgba(79, 195, 247, 0.5);
	}

	.fan-info {
		display: flex;
		align-items: baseline;
		gap: 6rpx;
	}

	.fan-speed {
		font-size: 42rpx;
		font-weight: 700;
		color: #4fc3f7;
	}

	.fan-unit {
		font-size: 22rpx;
		color: #888;
	}

	@keyframes fanSpin {
		0% { transform: rotate(0deg); }
		100% { transform: rotate(360deg); }
	}

	@keyframes fanBlade1 {
		0%, 100% { opacity: 0.85; transform: rotate(0deg) translateY(-28rpx); }
		50% { opacity: 1; transform: rotate(5deg) translateY(-30rpx); }
	}

	@keyframes fanBlade2 {
		0%, 100% { opacity: 0.85; transform: rotate(120deg) translateY(-28rpx); }
		50% { opacity: 1; transform: rotate(125deg) translateY(-30rpx); }
	}

	@keyframes fanBlade3 {
		0%, 100% { opacity: 0.85; transform: rotate(240deg) translateY(-28rpx); }
		50% { opacity: 1; transform: rotate(245deg) translateY(-30rpx); }
	}

	/* 转换效率全宽卡片 */
	.efficiency-full-card {
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 18rpx;
		padding: 22rpx 26rpx;
		border: 1px solid #ffd54f;
		margin-bottom: 14rpx;
	}

	.efficiency-body {
		display: flex;
		align-items: center;
		gap: 24rpx;
	}

	.efficiency-big-ring {
		flex-shrink: 0;
		position: relative;
		width: 116rpx;
		height: 116rpx;
		display: flex;
		align-items: center;
		justify-content: center;
	}

	.efficiency-ring-svg {
		width: 100%;
		height: 100%;
		transform: rotate(-90deg);
	}

	.efficiency-ring-fill {
		fill: none;
		stroke: #ffd54f;
		stroke-width: 3;
		stroke-linecap: round;
		transition: stroke-dasharray 0.6s ease;
	}

	.efficiency-ring-text {
		position: absolute;
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
	}

	.efficiency-big-value {
		font-size: 32rpx;
		font-weight: 700;
		color: #ffd54f;
		line-height: 1.1;
	}

	.efficiency-ring-unit {
		font-size: 18rpx;
		color: #888;
		line-height: 1;
	}

	/* 效率对比进度条 */
	.efficiency-bars {
		flex: 1;
		display: flex;
		flex-direction: column;
		gap: 12rpx;
	}

	.efficiency-bar-item {
		display: flex;
		flex-direction: column;
		gap: 6rpx;
	}

	.efficiency-bar-label {
		display: flex;
		justify-content: space-between;
		font-size: 22rpx;
		color: #888;
	}

	.efficiency-bar-val {
		font-weight: 600;
		color: #ccc;
	}

	.efficiency-bar-track {
		height: 12rpx;
		background: rgba(255, 255, 255, 0.05);
		border-radius: 6rpx;
		overflow: hidden;
	}

	.efficiency-bar-fill {
		height: 100%;
		border-radius: 6rpx;
		transition: width 0.5s ease;
	}

	.efficiency-bar-in {
		background: linear-gradient(90deg, #4fc3f7, #0288d1);
	}

	.efficiency-bar-out {
		background: linear-gradient(90deg, #ffd54f, #ff8f00);
	}
	/* 实时数据曲线卡片 */
	.chart-card {
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 18rpx;
		padding: 20rpx 24rpx;
		border: 1px solid #7c4dff;
		margin-bottom: 14rpx;
	}

	.chart-body {
		display: flex;
		flex-direction: column;
		gap: 10rpx;
	}

	.chart-legend {
		display: flex;
		gap: 20rpx;
		justify-content: center;
	}

	.chart-legend-item {
		display: flex;
		align-items: center;
		gap: 6rpx;
	}

	.chart-legend-dot {
		width: 12rpx;
		height: 12rpx;
		border-radius: 50%;
	}

	.chart-legend-label {
		font-size: 20rpx;
		color: #888;
	}

	.chart-svg-wrap {
		width: 100%;
		height: 130rpx;
		display: flex;
		align-items: center;
		justify-content: center;
	}

	.chart-svg {
		width: 100%;
		height: 100%;
	}

	.chart-gridline {
		stroke: rgba(255, 255, 255, 0.04);
		stroke-width: 1;
	}

	.chart-line {
		fill: none;
		stroke-width: 2;
		stroke-linejoin: round;
		stroke-linecap: round;
	}

	.chart-line-vout {
		stroke: #4fc3f7;
	}

	.chart-line-iout {
		stroke: #ff8a65;
	}

	.chart-line-wout {
		stroke: #ffd54f;
	}

	.chart-empty {
		display: flex;
		align-items: center;
		justify-content: center;
		height: 100%;
	}

	.chart-empty-text {
		font-size: 22rpx;
		color: #555;
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
