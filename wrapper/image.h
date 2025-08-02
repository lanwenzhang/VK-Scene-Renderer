#pragma once

#include "../common.h"
#include "device.h"
#include "command_pool.h"

namespace lzvk::wrapper {

	class Image {
	public:

		using Ptr = std::shared_ptr<Image>;

		static Image::Ptr createDepthImage(const Device::Ptr& device, const int& width, const int& height, VkSampleCountFlagBits sample);
		static Image::Ptr createRenderTargetImage(const Device::Ptr& device, const int& width, const int& height, VkFormat format);
	

	public:

		static Ptr create(const Device::Ptr& device, const int& width, const int& height, const VkFormat& format,
						  const VkImageType& imageType, const VkImageTiling& tiling, const VkImageUsageFlags& usage,
			              const VkSampleCountFlagBits& sample, const VkMemoryPropertyFlags& properties, const VkImageAspectFlags& aspectFlags,
						  const VkImageCreateFlags& imageCreateFlags = 0, const uint32_t& arrayLayers = 1, const VkImageViewType& viewType = VK_IMAGE_VIEW_TYPE_2D) {
			
			return std::make_shared<Image>(device, width, height, format, imageType, tiling, usage, sample, properties, aspectFlags,
										   imageCreateFlags, arrayLayers, viewType);
		}

		static Ptr create(const Device::Ptr& device, VkImage image, VkImageView imageView, VkFormat format) {

			return std::make_shared<Image>(device, image, imageView, format);
		}



		Image(const Device::Ptr& device, const int& width, const int& height, const VkFormat& format,
			  const VkImageType& imageType, const VkImageTiling& tiling, const VkImageUsageFlags& usage,
			  const VkSampleCountFlagBits& sample, const VkMemoryPropertyFlags& properties, const VkImageAspectFlags& aspectFlags,
			  const VkImageCreateFlags& imageCreateFlags = 0, const uint32_t& arrayLayers = 1, const VkImageViewType& viewType = VK_IMAGE_VIEW_TYPE_2D);
		
		Image(const Device::Ptr& device, VkImage image, VkImageView imageView, VkFormat format);

		~Image();


		void setImageLayout(VkImageLayout newLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresrouceRange,const CommandPool::Ptr& commandPool);
		void fillImageData(size_t size, void* pData, const CommandPool::Ptr& commandPool, uint32_t arrayLayer);
		
		
		void setImageLayout(VkImageLayout newLayout) { mLayout = newLayout; }

		[[nodiscard]] auto getImage() const { return mImage; }
		[[nodiscard]] auto getLayout() const { return mLayout; }
		[[nodiscard]] auto getFormat() const { return mFormat; }
		[[nodiscard]] auto getWidth() const { return mWidth; }
		[[nodiscard]] auto getHeight() const { return mHeight; }
		[[nodiscard]] auto getImageView() const { return mImageView; }

	public:

		static VkFormat findDepthFormat(const Device::Ptr& device);
		static VkFormat findSupportedFormat(const Device::Ptr& device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		bool hasStencilComponent(VkFormat format);

	private:

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);



	private:

		Device::Ptr			mDevice{ nullptr };
		VkImage				mImage{ VK_NULL_HANDLE };
		VkDeviceMemory		mImageMemory{ VK_NULL_HANDLE };
		VkImageView			mImageView{ VK_NULL_HANDLE };
		VkFormat			mFormat;
		VkImageLayout		mLayout{ VK_IMAGE_LAYOUT_UNDEFINED };

		size_t				mWidth{ 0 };
		size_t				mHeight{ 0 };
	};
}