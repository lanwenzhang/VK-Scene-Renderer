#pragma once

#include "../../common.h"
#include "../../wrapper/image.h"
#include "../../wrapper/sampler.h"
#include "../../wrapper/device.h"
#include "../../wrapper/commandPool.h"

namespace LZ::Renderer {

	class CubeMapTexture {
	public:

		using Ptr = std::shared_ptr<CubeMapTexture>;
		static Ptr create(const LZ::Wrapper::Device::Ptr& device, const LZ::Wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath) {

			return std::make_shared<CubeMapTexture>(device, commandPool, imageFilePath);
		};

		CubeMapTexture(const LZ::Wrapper::Device::Ptr& device, const LZ::Wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath);
		~CubeMapTexture();

		[[nodiscard]] VkDescriptorImageInfo& getImageInfo() { return mImageInfo; }
		[[nodiscard]] VkImageView getImageView() const { return mImageInfo.imageView; }
		[[nodiscard]] VkSampler getSampler() const { return mImageInfo.sampler; }

	private:

		LZ::Wrapper::Device::Ptr mDevice;
		LZ::Wrapper::CommandPool::Ptr mCommandPool;
		LZ::Wrapper::Image::Ptr mCubeMapImage;
		LZ::Wrapper::Sampler::Ptr mSampler;

		VkDescriptorImageInfo mImageInfo{};

	};
}