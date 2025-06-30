#pragma once

#include "../common.h"
#include "commandPool.h"
#include "device.h"
#include "../core/type.h"

namespace LZ::Wrapper {

	class CommandBuffer {
	public:
		using Ptr = std::shared_ptr<CommandBuffer>;
		static Ptr create(const Device::Ptr& device, const CommandPool::Ptr& commandPool, bool asSecondary = false) { 
			return std::make_shared<CommandBuffer>(device, commandPool, asSecondary); 
		}
		
		CommandBuffer(const Device::Ptr& device, const CommandPool::Ptr& commandPool, bool asSecondary = false);
		~CommandBuffer();

		// Command instructions
		void begin(VkCommandBufferUsageFlags flag = 0, const VkCommandBufferInheritanceInfo& inheritance = {});
		void beginRenderPass(const VkRenderPassBeginInfo& renderPassBeginInfo, const VkSubpassContents &subPassContents = VK_SUBPASS_CONTENTS_INLINE);

		void setViewport(uint32_t firstViewport, const VkViewport& viewport);
		void setScissor(uint32_t firstScissor, const VkRect2D& scissor);

		void bindGraphicPipeline(const VkPipeline& pipeline);
		void bindComputePipeline(const VkPipeline& pipeline);
		void bindVertexBuffer(const std::vector<VkBuffer>& buffers);
		void bindIndexBuffer(const VkBuffer& buffer);
		void bindDescriptorSet(const VkPipelineLayout layout, const VkDescriptorSet& descriptorSet, uint32_t setIndex);

		void pushConstants(const VkPipelineLayout layout, VkShaderStageFlags stageFlags, const LZ::Core::PushConstants& pc);
		void pushConstant(const VkPipelineLayout layout, VkShaderStageFlags stageFlags, const glm::mat4& mvp);
		void dispatch(uint32_t x, uint32_t y, uint32_t z);

		void draw(size_t vertexCount);
		void drawIndex(size_t indexCount);
		void drawIndexInstanced(uint32_t indexCount, uint32_t instancingCount);
		void drawIndexedIndirect(VkBuffer indirectBuffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);

		void endRenderPass();
		void end();

		void copyBufferToBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t copyInfoCount, const std::vector<VkBufferCopy>& copyInfos);
		void copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t width, uint32_t height, uint32_t arrayLayer = 0);

		void submitSync(VkQueue queue, VkFence fence = VK_NULL_HANDLE);

		void transferImageLayout(const VkImageMemoryBarrier &imageMemoryBarrier, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);

		[[nodiscard]] auto getCommandBuffer()const { return mCommandBuffer; }

	private:

		VkCommandBuffer mCommandBuffer{ VK_NULL_HANDLE };
		Device::Ptr mDevice{ nullptr };
		CommandPool::Ptr mCommandPool{ nullptr };

	};

}