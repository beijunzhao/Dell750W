<script>
	export default {
		onLaunch: function() {
			console.log('数控电源 App 启动')
			// 初始化蓝牙适配器
			// #ifdef APP-PLUS
			uni.openBluetoothAdapter({
				success: () => {
					console.log('蓝牙适配器就绪')
				},
				fail: () => {
					console.warn('蓝牙适配器不可用')
				}
			})
			// #endif
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
