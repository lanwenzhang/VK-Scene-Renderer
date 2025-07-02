#pragma once

#include "../common.h"
#include "device.h"

namespace lzvk::wrapper {

	class Sampler {
	public:
		using Ptr = std::shared_ptr<Sampler>;
		static Ptr create(const Device::Ptr& device) { return std::make_shared<Sampler>(device); }

		Sampler(const Device::Ptr& device);
		~Sampler();

		[[nodiscard]] auto getSampler() const { return mSampler; }

	private:
		Device::Ptr mDevice{ nullptr };
		VkSampler mSampler{ VK_NULL_HANDLE };
	};
}