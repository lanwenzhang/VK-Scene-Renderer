#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace lzvk::renderer {

	Texture::Texture(const lzvk::wrapper::Device::Ptr& device, const lzvk::wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath) {
		
		mDevice = device;

		// 1 load image
		int texWidth, texHeight, texSize, texChannles;
		stbi_uc* pixels = stbi_load(imageFilePath.c_str(), &texWidth, &texHeight, &texChannles, STBI_rgb_alpha);

		if (!pixels) {
			throw std::runtime_error("Error: failed to read image data");
		}

		texSize = texWidth * texHeight * 4;

		mImage = lzvk::wrapper::Image::create(
			mDevice, texWidth, texHeight,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT
		);

		VkImageSubresourceRange region{};
		region.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		region.baseArrayLayer = 0;
		region.layerCount = 1;

		region.baseMipLevel = 0;
		region.levelCount = 1;

		mImage->setImageLayout(
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			region,
			commandPool
		);

		mImage->fillImageData(texSize, (void*)pixels, commandPool, 0);

		mImage->setImageLayout(
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			region,
			commandPool
		);

		stbi_image_free(pixels);

		// create sampler
		mSampler = lzvk::wrapper::Sampler::create(mDevice);

		mImageInfo.imageLayout = mImage->getLayout();
		mImageInfo.imageView = mImage->getImageView();
		mImageInfo.sampler = mSampler->getSampler();
	}

	Texture::Texture(const lzvk::wrapper::Device::Ptr& device,
		const lzvk::wrapper::Image::Ptr& image)
	{
		mDevice = device;
		mImage = image;

		mSampler = lzvk::wrapper::Sampler::create(mDevice);
		mImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		mImageInfo.imageView = mImage->getImageView();
		mImageInfo.sampler = mSampler->getSampler();
	}

	Texture::~Texture() {

	}
}