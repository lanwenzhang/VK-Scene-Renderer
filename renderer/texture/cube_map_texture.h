#pragma once

#include "../../common.h"
#include "../../wrapper/image.h"
#include "../../wrapper/sampler.h"
#include "../../wrapper/device.h"
#include "../../wrapper/command_pool.h"

namespace lzvk::renderer {

	class CubeMapTexture {
	public:

		using Ptr = std::shared_ptr<CubeMapTexture>;
		static Ptr create(const lzvk::wrapper::Device::Ptr& device, const lzvk::wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath) {

			return std::make_shared<CubeMapTexture>(device, commandPool, imageFilePath);
		};

		CubeMapTexture(const lzvk::wrapper::Device::Ptr& device, const lzvk::wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath);
		~CubeMapTexture();

		[[nodiscard]] VkDescriptorImageInfo& getImageInfo() { return mImageInfo; }
		[[nodiscard]] VkImageView getImageView() const { return mImageInfo.imageView; }
		[[nodiscard]] VkSampler getSampler() const { return mImageInfo.sampler; }

	private:

		lzvk::wrapper::Device::Ptr mDevice;
		lzvk::wrapper::CommandPool::Ptr mCommandPool;
		lzvk::wrapper::Image::Ptr mCubeMapImage;
		lzvk::wrapper::Sampler::Ptr mSampler;

		VkDescriptorImageInfo mImageInfo{};

	};
}