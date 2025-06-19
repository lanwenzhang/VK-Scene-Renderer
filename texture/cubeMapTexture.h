#pragma once

#include "../base.h"
#include "../vulkanWrapper/image.h"
#include "../vulkanWrapper/sampler.h"
#include "../vulkanWrapper/device.h"
#include "../vulkanWrapper/commandPool.h"

namespace FF {

	class CubeMapTexture {
	public:

		using Ptr = std::shared_ptr<CubeMapTexture>;
		static Ptr create(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath) {

			return std::make_shared<CubeMapTexture>(device, commandPool, imageFilePath);
		};

		CubeMapTexture(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath);
		~CubeMapTexture();

		[[nodiscard]] VkDescriptorImageInfo& getImageInfo() { return mImageInfo; }

	private:

		Wrapper::Device::Ptr mDevice;
		Wrapper::CommandPool::Ptr mCommandPool;
		Wrapper::Image::Ptr mCubeMapImage;
		Wrapper::Sampler::Ptr mSampler;

		VkDescriptorImageInfo mImageInfo{};

	};
}