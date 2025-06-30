#include "descriptorPool.h"

namespace FF::Wrapper {

	DescriptorPool::DescriptorPool(const Device::Ptr& device) {
		
		mDevice = device;
	}

	DescriptorPool::~DescriptorPool() {

		if (mPool != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(mDevice->getDevice(), mPool, nullptr);
		}
	}

	void DescriptorPool::build(std::vector<UniformParameter::Ptr>& params, const int& frameCount) {

		// 1 Get the number of uniform buffer
		uint32_t uniformBufferCount = 0;
		uint32_t textureCount = 0;
		uint32_t storageBufferCount = 0;

		for (const auto& param : params) {
			if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
				uniformBufferCount += param->mCount;
			}
			if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
				textureCount += param->mCount;
			}
			if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
				storageBufferCount += param->mCount;
			}

		}

		// 2 Pool size
		std::vector<VkDescriptorPoolSize> poolSizes{};

		if (uniformBufferCount > 0) {
			VkDescriptorPoolSize uniformBufferSize{};
			uniformBufferSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uniformBufferSize.descriptorCount = uniformBufferCount * frameCount;
			poolSizes.push_back(uniformBufferSize);
		}

		if (textureCount > 0) {
			VkDescriptorPoolSize textureSize{};
			textureSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			textureSize.descriptorCount = textureCount * frameCount;
			poolSizes.push_back(textureSize);
		}

		if (storageBufferCount > 0) {
			VkDescriptorPoolSize storageSize{};
			storageSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			storageSize.descriptorCount = storageBufferCount * frameCount;
			poolSizes.push_back(storageSize);
		}

		if (poolSizes.empty()) {
			throw std::runtime_error("Error: DescriptorPool - no valid pool sizes.");
		}


		// 3 Pool create info
		VkDescriptorPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		createInfo.pPoolSizes = poolSizes.data();
		createInfo.maxSets = static_cast<uint32_t>(frameCount) + 1;
		createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

		// 4 Create pool
		if (vkCreateDescriptorPool(mDevice->getDevice(), &createInfo, nullptr, &mPool) != VK_SUCCESS) {

			throw std::runtime_error("Error: failed to create Descriptor pool");
		}
	}

}