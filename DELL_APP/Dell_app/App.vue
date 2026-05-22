<script>
	import bleService from './utils/ble-service.js'

	export default {
		onLaunch: function() {
			console.log('数控电源 App 启动')
			// 全局只初始化一次蓝牙（各页面不再重复调用）
			bleService.init()
			// 初始化后自动重连上次连接的设备
			setTimeout(() => {
				bleService.autoReconnect()
			}, 1000)  // 延迟1秒，等待蓝牙适配器初始化完成
		},
		onShow: function() {
			console.log('App Show')
		},
		onHide: function() {
			console.log('App Hide')
		}
	}
</script>

<style>
	/* 每个页面公共 CSS */
	/* 全局滚动条样式 */
	::-webkit-scrollbar {
		width: 0;
		height: 0;
	}

	/* 按钮统一样式重置 */
	button {
		padding: 0;
		margin: 0;
		line-height: 1;
	}

	button::after {
		border: none;
	}

	/* 滑块统一样式 */
	uni-slider .uni-slider-handle-wrapper {
		height: 8rpx !important;
	}

	/* 全局字体 */
	page {
		font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;
		-webkit-font-smoothing: antialiased;
		-moz-osx-font-smoothing: grayscale;
	}

	/* 过渡动画 */
	.fade-enter-active,
	.fade-leave-active {
		transition: opacity 0.3s;
	}
	.fade-enter,
	.fade-leave-to {
		opacity: 0;
	}
</style>
