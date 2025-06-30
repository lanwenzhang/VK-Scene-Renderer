#pragma once

#include "../../common.h"
#include "../../wrapper/image.h"
#include "../../wrapper/sampler.h"
#include "../../wrapper/device.h"
#include "../../wrapper/commandPool.h"


namespace LZ::Renderer {

	class Texture {
	public:
		using Ptr = std::shared_ptr<Texture>;
		static Ptr create(const LZ::Wrapper::Device::Ptr& device, const LZ::Wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath) {
			return std::make_shared<Texture>(device, commandPool, imageFilePath);
		}

		Texture(const LZ::Wrapper::Device::Ptr& device, const LZ::Wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath);

		~Texture();	

		[[nodiscard]] auto getImage() const { return mImage; }
		[[nodiscard]] VkDescriptorImageInfo& getImageInfo() { return mImageInfo; }
		[[nodiscard]] VkImageView getImageView() const { return mImageInfo.imageView; }
		[[nodiscard]] VkSampler getSampler() const { return mImageInfo.sampler; }

	private:

		LZ::Wrapper::Device::Ptr mDevice{ nullptr };
		LZ::Wrapper::Image::Ptr mImage{ nullptr };
		LZ::Wrapper::Sampler::Ptr mSampler{ nullptr };
		VkDescriptorImageInfo mImageInfo{};
	};

}