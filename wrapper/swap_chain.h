#pragma once

#include "../common.h"
#include "../core/window.h"
#include "device.h"
#include "render_pass.h"
#include "image.h"

namespace lzvk::wrapper {

	struct SwapChainSupportInfo {

		VkSurfaceCapabilitiesKHR mCapabilities;
		std::vector<VkSurfaceFormatKHR> mFormats;
		std::vector<VkPresentModeKHR> mPresentModes;

	};

	class SwapChain {
	public:

		SwapChain(const Device::Ptr &device, const lzvk::core::Window::Ptr &window, const Surface::Ptr &surface, const CommandPool::Ptr& commandPool);
		~SwapChain();

		using Ptr = std::shared_ptr<SwapChain>;
		static Ptr create(const Device::Ptr& device, const lzvk::core::Window::Ptr& window, const Surface::Ptr& surface, const CommandPool::Ptr& commandPool) {
			
			return std::make_shared<SwapChain>(device, window, surface, commandPool);

		}

		SwapChainSupportInfo querySwapChainSupportInfo();

		VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		VkPresentModeKHR chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR & capabilities);

		void createFrameBuffers(const RenderPass::Ptr &renderPass);

		[[nodiscard]] auto getFormat() const { return mSwapChainFormat; }
		[[nodiscard]] auto getImageCount() const { return mImageCount; }
		[[nodiscard]] auto getSwapChain() const { return mSwapChain; }
		[[nodiscard]] auto getFrameBuffer(const int index) const { return mSwapChainFrameBuffers[index]; }
		[[nodiscard]] auto getExtent() const { return mSwapChainExtent; }


	private:

		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels = 1);


	private:

		VkSwapchainKHR mSwapChain{ VK_NULL_HANDLE };
		VkFormat mSwapChainFormat;
		VkExtent2D mSwapChainExtent;

		// Swap chain images
		uint32_t mImageCount{ 0 };
		std::vector<VkImage> mSwapChainImages{};
		std::vector<VkImageView> mSwapChainImageViews{}; // Manage swap chain image
		std::vector<Image::Ptr> mDepthImages{};
		std::vector<Image::Ptr> mMultiSampleImages{}; 
		std::vector<VkFramebuffer> mSwapChainFrameBuffers{};

		Device::Ptr mDevice{ nullptr };
		lzvk::core::Window::Ptr mWindow{ nullptr };
		Surface::Ptr mSurface{ nullptr };

	};
}