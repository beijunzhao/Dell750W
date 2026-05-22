<template>
	<view class="container">

		<!-- 设备信息 -->
		<view class="section">
			<view class="section-header">
				<text class="section-icon">ℹ️</text>
				<text class="section-title">设备信息</text>
			</view>
			<view class="info-list">
				<view class="info-row">
					<text class="info-key">厂商 ID</text>
					<text class="info-value">{{ deviceInfo.mfr_id || '--' }}</text>
				</view>
				<view class="info-row">
					<text class="info-key">型号</text>
					<text class="info-value">{{ deviceInfo.mfr_model || '--' }}</text>
				</view>
				<view class="info-row">
					<text class="info-key">版本</text>
					<text class="info-value">{{ deviceInfo.mfr_revision || '--' }}</text>
				</view>
				<view class="info-row">
					<text class="info-key">产地</text>
					<text class="info-value">{{ deviceInfo.mfr_location || '--' }}</text>
				</view>
				<view class="info-row">
					<text class="info-key">生产日期</text>
					<text class="info-value">{{ deviceInfo.mfr_date || '--' }}</text>
				</view>
				<view class="info-row">
					<text class="info-key">序列号</text>
					<text class="info-value">{{ deviceInfo.mfr_serial || '--' }}</text>
				</view>
			</view>
		</view>

		<!-- 电源规格 -->
		<view class="section">
			<view class="section-header">
				<text class="section-icon">📋</text>
				<text class="section-title">电源规格</text>
			</view>
			<view class="info-list">
				<view class="info-row">
					<text class="info-key">最大输出电压</text>
					<text class="info-value">{{ voltageMax }}V</text>
				</view>
				<view class="info-row">
					<text class="info-key">最大输出电流</text>
					<text class="info-value">62.5A</text>
				</view>
				<view class="info-row">
					<text class="info-key">最大输出功率</text>
					<text class="info-value">750W</text>
				</view>
			</view>
		</view>

		<!-- 关于 -->
		<view class="section">
			<view class="section-header">
				<text class="section-icon">📱</text>
				<text class="section-title">关于</text>
			</view>
			<view class="info-list">
				<view class="info-row">
					<text class="info-key">App 名称</text>
					<text class="info-value">数控电源</text>
				</view>
				<view class="info-row">
					<text class="info-key">版本</text>
					<text class="info-value">1.0.0</text>
				</view>
				<view class="info-row">
					<text class="info-key">硬件平台</text>
					<text class="info-value">ESP32-C3</text>
				</view>
				<view class="info-row">
					<text class="info-key">适用电源</text>
					<text class="info-value">戴尔750W服务器电源</text>
				</view>
			</view>
		</view>

		<!-- 刷新数据按钮 -->
		<view class="action-section">
			<button class="refresh-btn" :disabled="!connected" @tap="refreshData">
				🔄 刷新数据
			</button>
		</view>

		<!-- 底部导航 -->
		<view class="bottom-nav">
			<view class="nav-item" @tap="switchTab('index')">
				<text class="nav-icon">📊</text>
				<text class="nav-label">监控</text>
			</view>
			<view class="nav-item" @tap="switchTab('control')">
				<text class="nav-icon">🎛️</text>
				<text class="nav-label">控制</text>
			</view>
			<view class="nav-item active" @tap="switchTab('settings')">
				<text class="nav-icon">ℹ️</text>
				<text class="nav-label">信息</text>
			</view>
			<!-- 蓝牙状态 -->
			<view class="nav-ble" :class="{ connected: connected }" @tap="disconnect">
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
				statusText: '未连接',
				voltageMax: 12.0,
				currentMax: 60.0,
				deviceInfo: {
					mfr_id: '',
					mfr_model: '',
					mfr_revision: '',
					mfr_location: '',
					mfr_date: '',
					mfr_serial: '',
					pmbus_revision: ''
				},
				/** 防抖定时器：防止 _requestInfo() 被频繁调用 */
				_infoDebounceTimer: null
			}
		},
		onLoad() {
			// 蓝牙已在 App.vue onLaunch 中全局初始化，此处不再重复调用

			bleService.onStatus((status) => {
				this.statusText = status
				this.connected = bleService.connected
				// 只在真正"已连接"状态时请求设备信息
				// 过滤掉"蓝牙就绪"、"正在扫描..."、"连接成功，发现服务..."等中间状态
				if (bleService.connected && status === '已连接') {
					this._requestInfoDebounced()
				}
			})

			bleService.onData((data) => {
				// 更新设备信息
				if (data.MFR_ID) {
					this.deviceInfo.mfr_id = this._parseMfrId(data.MFR_ID)
				}
				if (data.MFR_MODEL) this.deviceInfo.mfr_model = data.MFR_MODEL
				if (data.MFR_REVISION) this.deviceInfo.mfr_revision = data.MFR_REVISION
				if (data.MFR_LOCATION) this.deviceInfo.mfr_location = data.MFR_LOCATION
				if (data.MFR_DATE) {
					// 解析生产日期：PMBus 格式为 YYWW（年+周），如 "15100"=15年第100周
					// 台达电源使用 5 位格式：YY + WWW（3位周数）
					this.deviceInfo.mfr_date = this._parseMfrDate(data.MFR_DATE)
				}
				if (data.MFR_SERIAL) this.deviceInfo.mfr_serial = data.MFR_SERIAL
				if (data.pmbus_revision !== undefined) {
					const rev = data.pmbus_revision
					this.deviceInfo.pmbus_revision = `${Math.floor(rev / 10)}.${rev % 10}`
				}
			})

			// 同步连接状态（连接状态变化由 ble-service.js 全局监听处理）
			this.connected = bleService.connected
			this.statusText = bleService.connected ? '已连接' : '未连接'
			// 如果已连接，立即请求设备信息（不防抖，让用户秒看到数据）
			if (this.connected) {
				this._requestInfoImmediate()
			}
		},
		onShow() {
			// 每次页面显示时同步连接状态
			this.connected = bleService.connected
			this.statusText = bleService.connected ? '已连接' : '未连接'
			// 如果已连接且还没有设备信息，立即请求（不防抖）
			if (this.connected && !this.deviceInfo.mfr_id) {
				this._requestInfoImmediate()
			}
		},
		onUnload() {
			// 页面卸载时清除防抖定时器
			if (this._infoDebounceTimer) {
				clearTimeout(this._infoDebounceTimer)
				this._infoDebounceTimer = null
			}
		},
		methods: {

			/**
			 * 根据 MFR_ID 字符串解析厂商名称
			 * 不同电源厂商的 MFR_ID 格式不同，通过前缀/关键字匹配识别
			 * @param {string} id PMBus 读取的原始 MFR_ID
			 * @returns {string} 解析后的厂商显示名称
			 */
			_parseMfrId(id) {
				if (!id) return '--'
				const upper = id.toUpperCase().trim()
				// 厂商识别映射表（按优先级排列）
				if (upper.includes('DELTA') || upper.includes('DPS-')) return '台达 (Delta)'
				if (upper.includes('ARTESYN')) return 'Artesyn (安森美)'
				if (upper.includes('FLEX') || upper.includes('FLEXTRONICS')) return 'Flex (伟创力)'
				if (upper.includes('LITEON') || upper.includes('LITE-ON')) return '光宝 (Lite-On)'
				if (upper.includes('ACBEL')) return '康舒 (AcBel)'
				if (upper.includes('CHICONY')) return '群光 (Chicony)'
				if (upper.includes('HIPRO')) return '高效 (HiPro)'
				// 戴尔 PN 格式：以数字开头，如 09PXCVA01PS
				if (/^\d{2}[A-Z]/.test(upper)) return '台达 (Delta)'  // 戴尔电源多为台达代工
				// 未知厂商，返回原始值
				return id
			},

			/**
			 * 解析 PMBus 生产日期格式 (YYWWW) 为年月
			 * 台达电源使用 5 位格式：前 2 位年份 + 后 3 位周数
			 * 例如 "15100" → 2015 年第 100 周 → 约 2015-12
			 * @param {string} dateStr PMBus 原始日期字符串
			 * @returns {string} 格式化后的年月字符串
			 */
			_parseMfrDate(dateStr) {
				if (!dateStr) return '--'
				// 尝试匹配 YYWWW 格式（5位数字）
				const match = dateStr.trim().match(/^(\d{2})(\d{2,3})$/)
				if (!match) return dateStr  // 无法解析则返回原始值

				const year = 2000 + parseInt(match[1])
				const week = parseInt(match[2])
				// 将周数估算为月份：week / 52 * 12，四舍五入
				let month = Math.round(week / 52 * 12)
				if (month < 1) month = 1
				if (month > 12) month = 12
				return `${year}-${String(month).padStart(2, '0')}`
			},

			/**
			 * 立即发送设备信息请求（不防抖）
			 * 用于页面加载/显示时的主动请求，让用户秒看到数据
			 */
			_requestInfoImmediate() {
				if (!this.connected) return
				// 清除防抖定时器，避免防抖版本再发一次
				if (this._infoDebounceTimer) {
					clearTimeout(this._infoDebounceTimer)
					this._infoDebounceTimer = null
				}
				bleService.getInfo().catch(err => {
					console.error('[Settings] 获取设备信息失败:', err)
				})
			},

			/**
			 * 防抖请求设备信息（静默模式，不显示 loading）
			 * 用于 onStatus 回调，防止连接过程中多次状态通知触发重复请求
			 * 500ms 防抖：短时间内多次调用只发送一次
			 */
			_requestInfoDebounced() {
				if (!this.connected) return
				// 清除之前的防抖定时器
				if (this._infoDebounceTimer) {
					clearTimeout(this._infoDebounceTimer)
				}
				// 设置新的防抖定时器：500ms 内只执行最后一次
				this._infoDebounceTimer = setTimeout(() => {
					this._infoDebounceTimer = null
					bleService.getInfo().catch(err => {
						console.error('[Settings] 获取设备信息失败:', err)
					})
				}, 500)
			},

			async refreshData() {
				if (!this.connected) return
				// 手动刷新不受防抖限制，直接发送
				if (this._infoDebounceTimer) {
					clearTimeout(this._infoDebounceTimer)
					this._infoDebounceTimer = null
				}
				uni.showLoading({ title: '获取数据...' })
				try {
					await bleService.getInfo()
					setTimeout(() => {
						uni.hideLoading()
						uni.showToast({ title: '已请求数据', icon: 'success' })
					}, 500)
				} catch (err) {
					uni.hideLoading()
					uni.showToast({ title: '请求失败', icon: 'error' })
				}
			},

			disconnect() {
				bleService.disconnect()
				this.connected = false
				this.statusText = '已断开'
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

	/* 分区 */
	.section {
		background: linear-gradient(135deg, #1a1a3e, #16213e);
		border-radius: 20rpx;
		padding: 30rpx;
		margin-bottom: 24rpx;
		border: 1px solid #2a2a5e;
	}

	.section-header {
		display: flex;
		align-items: center;
		margin-bottom: 24rpx;
		padding-bottom: 16rpx;
		border-bottom: 1px solid #2a2a5e;
	}

	.section-icon {
		font-size: 32rpx;
		margin-right: 12rpx;
	}

	.section-title {
		font-size: 28rpx;
		font-weight: 600;
		color: #e0e0e0;
	}

	/* 信息列表 */
	.info-list {
		display: flex;
		flex-direction: column;
		gap: 16rpx;
	}

	.info-row {
		display: flex;
		justify-content: space-between;
		align-items: center;
	}

	.info-key {
		font-size: 26rpx;
		color: #888;
	}

	.info-value {
		font-size: 26rpx;
		color: #e0e0e0;
		text-align: right;
		max-width: 60%;
	}

	/* 操作区域 */
	.action-section {
		margin-top: 20rpx;
	}

	.refresh-btn {
		width: 100%;
		padding: 28rpx;
		background: linear-gradient(135deg, #4fc3f7, #0288d1);
		color: #fff;
		border-radius: 16rpx;
		font-size: 30rpx;
		border: none;
	}

	.refresh-btn[disabled] {
		opacity: 0.4;
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
