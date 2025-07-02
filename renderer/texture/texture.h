#pragma once

#include "../../common.h"
#include "../../wrapper/image.h"
#include "../../wrapper/sampler.h"
#include "../../wrapper/device.h"
#include "../../wrapper/command_pool.h"


namespace lzvk::renderer {

	class Texture {
	public:
		using Ptr = std::shared_ptr<Texture>;
		static Ptr create(const lzvk::wrapper::Device::Ptr& device, const lzvk::wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath) {
			return std::make_shared<Texture>(device, commandPool, imageFilePath);
		}

		Texture(const lzvk::wrapper::Device::Ptr& device, const lzvk::wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath);

		~Texture();	

		[[nodiscard]] auto getImage() const { return mImage; }
		[[nodiscard]] VkDescriptorImageInfo& getImageInfo() { return mImageInfo; }
		[[nodiscard]] VkImageView getImageView() const { return mImageInfo.imageView; }
		[[nodiscard]] VkSampler getSampler() const { return mImageInfo.sampler; }

	private:

		lzvk::wrapper::Device::Ptr mDevice{ nullptr };
		lzvk::wrapper::Image::Ptr mImage{ nullptr };
		lzvk::wrapper::Sampler::Ptr mSampler{ nullptr };
		VkDescriptorImageInfo mImageInfo{};
	};

}