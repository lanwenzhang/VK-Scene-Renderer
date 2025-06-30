#pragma once

#include "../base.h"
#include "../vulkanWrapper/image.h"
#include "../vulkanWrapper/sampler.h"
#include "../vulkanWrapper/device.h"
#include "../vulkanWrapper/commandPool.h"

namespace FF {

	class Texture {
	public:
		using Ptr = std::shared_ptr<Texture>;
		static Ptr create(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath) {
			return std::make_shared<Texture>(device, commandPool, imageFilePath);
		}

		Texture(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath);

		~Texture();	

		[[nodiscard]] auto getImage() const { return mImage; }
		[[nodiscard]] VkDescriptorImageInfo& getImageInfo() { return mImageInfo; }
		[[nodiscard]] VkImageView getImageView() const { return mImageInfo.imageView; }
		[[nodiscard]] VkSampler getSampler() const { return mImageInfo.sampler; }

	private:

		Wrapper::Device::Ptr mDevice{ nullptr };
		Wrapper::Image::Ptr mImage{ nullptr };
		Wrapper::Sampler::Ptr mSampler{ nullptr };
		VkDescriptorImageInfo mImageInfo{};
	};

}