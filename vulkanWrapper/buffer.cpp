#include "buffer.h"
#include "commandBuffer.h"
#include "commandPool.h"

namespace FF::Wrapper {

	Buffer::Ptr Buffer::createVertexBuffer(const Device::Ptr& device, VkDeviceSize size, const void* pData) {
		
		auto buffer = Buffer::create(
			device, size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		buffer->updateBufferByStage(pData, size);

		return buffer;
	}

	Buffer::Ptr Buffer::createIndexBuffer(const Device::Ptr& device, VkDeviceSize size, const void* pData) {
		
		auto buffer = Buffer::create(
			device, size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		buffer->updateBufferByStage(pData, size);

		return buffer;
	}

	Buffer::Ptr Buffer::createUniformBuffer(const Device::Ptr& device, VkDeviceSize size, const void* pData) {
		
		auto buffer = Buffer::create(device, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (pData != nullptr) {

			buffer->updateBufferByStage(pData, size);
		}

		return buffer;
	}

	Buffer::Ptr Buffer::createStageBuffer(const Device::Ptr& device, VkDeviceSize size, const void* pData) {

		auto buffer = Buffer::create(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (pData != nullptr) {

			buffer->updateBufferByMap(pData, size);
		}

		return buffer;
	}

	Buffer::Ptr Buffer::createStorageBuffer(const Device::Ptr& device, VkDeviceSize size, const void* pData, bool supportDeviceAddress) {

		VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

		if (supportDeviceAddress) {

			usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		}

		auto buffer = create(device, size, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, supportDeviceAddress); 

		if (pData != nullptr) {

			buffer->updateBufferByStage(pData, size);

		}

		return buffer;
	}

	Buffer::Buffer(const Device::Ptr& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool supportDeviceAddress){
	
		mDevice = device;

		// 1 Buffer request from CPU
		// 1.1 Buffer create info
		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = size;
		createInfo.usage = usage;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		// 1.2 Create buffer
		if (vkCreateBuffer(mDevice->getDevice(), &createInfo, nullptr, &mBuffer) != VK_SUCCESS) {
			
			throw std::runtime_error("Error:failed to create buffer");
		}

		// 2 Allocate buffer in GPU memory
		// 2.1 Memory create info
		VkMemoryRequirements memReq{};
		vkGetBufferMemoryRequirements(mDevice->getDevice(), mBuffer, &memReq);

		VkMemoryAllocateFlagsInfo flagsInfo{};
		flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		flagsInfo.flags = supportDeviceAddress ? VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT : 0;

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties);
		allocInfo.pNext = supportDeviceAddress ? &flagsInfo : nullptr;

		// 2.2 Allocate memory
		if (vkAllocateMemory(mDevice->getDevice(), &allocInfo, nullptr, &mBufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to allocate memory");
		}

		// 3 Bind buffer handle with buffer memory
		vkBindBufferMemory(mDevice->getDevice(), mBuffer, mBufferMemory, 0);

		mBufferInfo.buffer = mBuffer;
		mBufferInfo.offset = 0;
		mBufferInfo.range = size;
	}
	
	uint32_t Buffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {


		VkPhysicalDeviceMemoryProperties memProps;
		vkGetPhysicalDeviceMemoryProperties(mDevice->getPhysicalDevice(), &memProps);

		for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {

			// Requirement 1: meet the type
			// Requirement 2: meet the flag of type
			if ((typeFilter & (1 << i)) && ((memProps.memoryTypes[i].propertyFlags & properties) == properties)) {
				
				return i;
			}
		}

		throw std::runtime_error("Error: cannot find the property memory type!");
	}

	void Buffer::updateBufferByMap(const void* data, size_t size) {
		
		void* memPtr = nullptr;

		vkMapMemory(mDevice->getDevice(), mBufferMemory, 0, size, 0, &memPtr);
		memcpy(memPtr, data, size);
		vkUnmapMemory(mDevice->getDevice(), mBufferMemory);

	}

	void Buffer::updateBufferByStage(const void* data, size_t size) {
		
		auto stageBuffer = Buffer::create(mDevice, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stageBuffer->updateBufferByMap(data, size);

		copyBuffer(stageBuffer->getBuffer(), mBuffer, static_cast<VkDeviceSize>(size));
	}

	void Buffer::updateBufferByStage(const void* data, size_t size, size_t offset) {
		auto stageBuffer = Buffer::create(
			mDevice,
			size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		stageBuffer->updateBufferByMap(data, size);

		copyBuffer(stageBuffer->getBuffer(), mBuffer, static_cast<VkDeviceSize>(size), static_cast<VkDeviceSize>(offset));
	}

	void Buffer::copyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size) {
		
		auto commandPool = CommandPool::create(mDevice);
		auto commandBuffer = CommandBuffer::create(mDevice, commandPool);

		commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VkBufferCopy copyInfo{};
		copyInfo.size = size;

		commandBuffer->copyBufferToBuffer(srcBuffer, dstBuffer, 1, { copyInfo });

		commandBuffer->end();

		commandBuffer->submitSync(mDevice->getGraphicQueue(), VK_NULL_HANDLE);
	}

	void Buffer::copyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size, VkDeviceSize dstOffset) {

		auto commandPool = CommandPool::create(mDevice);
		auto commandBuffer = CommandBuffer::create(mDevice, commandPool);

		commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VkBufferCopy copyInfo{};
		copyInfo.srcOffset = 0;
		copyInfo.dstOffset = dstOffset;
		copyInfo.size = size;

		commandBuffer->copyBufferToBuffer(srcBuffer, dstBuffer, 1, { copyInfo });

		commandBuffer->end();
		commandBuffer->submitSync(mDevice->getGraphicQueue(), VK_NULL_HANDLE);
	}

	Buffer::~Buffer(){
	
		if (mBuffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(mDevice->getDevice(), mBuffer, nullptr);
		}

		if (mBufferMemory != VK_NULL_HANDLE) {
			vkFreeMemory(mDevice->getDevice(), mBufferMemory, nullptr);
		}
	}

}