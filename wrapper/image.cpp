#include "image.h"
#include "command_buffer.h"
#include "buffer.h"

namespace lzvk::wrapper {

	Image::Ptr Image::createDepthImage(const Device::Ptr& device, const int& width, const int& height, VkSampleCountFlagBits sample) {

		std::vector<VkFormat> formats = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,VK_FORMAT_D24_UNORM_S8_UINT, };

		VkFormat resultFormat = findSupportedFormat(device, formats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

		return Image::create(device, width, height, resultFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, 
			                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,sample, 
							 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT,
			                 0, 1, VK_IMAGE_VIEW_TYPE_2D);

	}

	Image::Ptr Image::createRenderTargetImage(const Device::Ptr& device, const int& width, const int& height, VkFormat format) {
		
		return Image::create(device, width, height, format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
							 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, device->getMaxUsableSampleCount(),
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
			                 0, 1, VK_IMAGE_VIEW_TYPE_2D);

	}


	Image::Image(const Device::Ptr& device, const int& width, const int& height, const VkFormat& format,
		const VkImageType& imageType, const VkImageTiling& tiling, const VkImageUsageFlags& usage,
		const VkSampleCountFlagBits& sample, const VkMemoryPropertyFlags& properties, const VkImageAspectFlags& aspectFlags,
		const VkImageCreateFlags& imageCreateFlags, const uint32_t& arrayLayers, const VkImageViewType& viewType) {

		mDevice = device;
		mLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		mWidth = width;
		mHeight = height;
		mFormat = format;

		// 1 Image create info
		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.extent.width = width;
		imageCreateInfo.extent.height = height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.format = format;
		imageCreateInfo.imageType = imageType;
		imageCreateInfo.tiling = tiling;
		imageCreateInfo.usage = usage;
		imageCreateInfo.samples = sample;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = arrayLayers;
		imageCreateInfo.flags = imageCreateFlags;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(mDevice->getDevice(), &imageCreateInfo, nullptr, &mImage) != VK_SUCCESS) {

			throw std::runtime_error("Error:failed to create image");

		}

		// 2 Allocate memory
		VkMemoryRequirements memReq{};
		vkGetImageMemoryRequirements(mDevice->getDevice(), mImage, &memReq);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;

		allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties);

		if (vkAllocateMemory(mDevice->getDevice(), &allocInfo, nullptr, &mImageMemory) != VK_SUCCESS) {

			throw std::runtime_error("Error: failed to allocate memory");

		}

		vkBindImageMemory(mDevice->getDevice(), mImage, mImageMemory, 0);

		// 3 Image view create info
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = viewType;
		imageViewCreateInfo.format = format;
		imageViewCreateInfo.image = mImage;
		imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = arrayLayers;

		imageViewCreateInfo.components = {
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY
		};

		if (vkCreateImageView(mDevice->getDevice(), &imageViewCreateInfo, nullptr, &mImageView) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create image view");
		}
	}


	Image::Image(const Device::Ptr& device, VkImage image, VkImageView imageView, VkFormat format) {

		mDevice = device;
		mImage = image;
		mImageView = imageView;
		mFormat = format;
	}


	Image::~Image() {

		if (mImageView != VK_NULL_HANDLE) {

			vkDestroyImageView(mDevice->getDevice(), mImageView, nullptr);
		}

		if (mImageMemory != VK_NULL_HANDLE) {

			vkFreeMemory(mDevice->getDevice(), mImageMemory, nullptr);
		}

		if (mImage != VK_NULL_HANDLE) {

			vkDestroyImage(mDevice->getDevice(), mImage, nullptr);
		}
	}

	uint32_t Image::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {

		VkPhysicalDeviceMemoryProperties memProps;
		vkGetPhysicalDeviceMemoryProperties(mDevice->getPhysicalDevice(), &memProps);

		for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {

			if ((typeFilter & (1 << i)) && ((memProps.memoryTypes[i].propertyFlags & properties) == properties)) {
				return i;
			}
		}

		throw std::runtime_error("Error: cannot find the property memory type!");
	}

	VkFormat Image::findDepthFormat(const Device::Ptr& device) {

		std::vector<VkFormat> formats = { VK_FORMAT_D32_SFLOAT,VK_FORMAT_D32_SFLOAT_S8_UINT,VK_FORMAT_D24_UNORM_S8_UINT, };

		return findSupportedFormat(device, formats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	}

	VkFormat Image::findSupportedFormat(const Device::Ptr& device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {

		for (auto format : candidates) {

			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(device->getPhysicalDevice(), format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {

				return format;

			}

			if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {

				return format;
			}
		}

		throw std::runtime_error("Error: can not find proper format");

	}

	bool Image::hasStencilComponent(VkFormat format) {

		return mFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || mFormat == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	void Image::setImageLayout(VkImageLayout newLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresrouceRange, const CommandPool::Ptr& commandPool) {

		// 1 Create barrier
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.oldLayout = mLayout;
		imageMemoryBarrier.newLayout = newLayout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = mImage;
		imageMemoryBarrier.subresourceRange = subresrouceRange;

		// 2 Check layout
		switch (mLayout)
		{
			// 2.1 Undefined to transfer destination: transfer writes that don't need to wait on anything
		case VK_IMAGE_LAYOUT_UNDEFINED:
			imageMemoryBarrier.srcAccessMask = 0;
			break;
			// 2.2 Transfer destination to shader reading: shader reads should wait on transfer writes
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		//case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		//	imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		//	break;
		default:
			break;
		}


		switch (newLayout)
		{
			// 2.1 Undefined to transfer destination: transfer writes that don't need to wait on anything
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

			// 2.2 Transfer destination to shader reading: shader reads should wait on transfer writes
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: {

			if (imageMemoryBarrier.srcAccessMask == 0) {

				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}

			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: {

			imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: {

			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}
			break;
		default:
			break;
		}

		// 3 Set new layout as current layout
		if (subresrouceRange.baseArrayLayer == 0 && subresrouceRange.layerCount == 6) {
			mLayout = newLayout; // Full cubemap transition
		}
		else if (subresrouceRange.baseArrayLayer == 0 && subresrouceRange.layerCount == 1) {
			mLayout = newLayout; // Ordinary 2D texture
		}

		// 4 Command
		auto commandBuffer = CommandBuffer::create(mDevice, commandPool);
		commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		commandBuffer->transferImageLayout(imageMemoryBarrier, srcStageMask, dstStageMask);
		commandBuffer->end();

		commandBuffer->submitSync(mDevice->getGraphicQueue());
	}

	void Image::fillImageData(size_t size, void* pData, const CommandPool::Ptr& commandPool, uint32_t arrayLayer) {

		assert(pData);
		assert(size);

		auto stageBuffer = Buffer::createStageBuffer(mDevice, size, pData);

		auto commandBuffer = CommandBuffer::create(mDevice, commandPool);
		commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		commandBuffer->copyBufferToImage(stageBuffer->getBuffer(), mImage, mLayout, mWidth, mHeight, arrayLayer);
		commandBuffer->end();

		commandBuffer->submitSync(mDevice->getGraphicQueue());
	}

}