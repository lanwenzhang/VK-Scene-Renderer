#include "command_buffer.h"

namespace lzvk::wrapper {


	CommandBuffer::CommandBuffer(const Device::Ptr& device, const CommandPool::Ptr& commandPool, bool asSecondary){
		
		mDevice = device;
		mCommandPool = commandPool;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = mCommandPool->getCommandPool();
		allocInfo.level = asSecondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		if (vkAllocateCommandBuffers(mDevice->getDevice(), &allocInfo, &mCommandBuffer) != VK_SUCCESS) {

			throw std::runtime_error("Error: failed to create command buffer");
		}
	}

	CommandBuffer::~CommandBuffer() {

		if (mCommandBuffer != VK_NULL_HANDLE) {

			vkFreeCommandBuffers(mDevice->getDevice(), mCommandPool->getCommandPool(), 1, &mCommandBuffer);
		}
	}

	void CommandBuffer::begin(VkCommandBufferUsageFlags flag, const VkCommandBufferInheritanceInfo& inheritance){
	
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = flag;
		beginInfo.pInheritanceInfo = &inheritance;

		if (vkBeginCommandBuffer(mCommandBuffer, &beginInfo) != VK_SUCCESS) {

			throw std::runtime_error("Error: failed to begin command buffer");
		};
	}

	void CommandBuffer::beginRenderPass(const VkRenderPassBeginInfo& renderPassBeginInfo, const VkSubpassContents& subPassContents){
	
		vkCmdBeginRenderPass(mCommandBuffer, &renderPassBeginInfo, subPassContents);
	
	}

	void CommandBuffer::setViewport(uint32_t firstViewport, const VkViewport& viewport){
		
		vkCmdSetViewport(mCommandBuffer, firstViewport, 1, &viewport);
	}

	void CommandBuffer::setScissor(uint32_t firstScissor, const VkRect2D& scissor){
		vkCmdSetScissor(mCommandBuffer, firstScissor, 1, &scissor);
	}


	void CommandBuffer::bindGraphicPipeline(const VkPipeline& pipeline){
	
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	void CommandBuffer::bindComputePipeline(const VkPipeline& pipeline) {
		
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	}

	void CommandBuffer::pushConstants(const VkPipelineLayout layout, VkShaderStageFlags stageFlags, const lzvk::core::PushConstants& pc) {
		
		vkCmdPushConstants(mCommandBuffer, layout, stageFlags, 0, sizeof(lzvk::core::PushConstants), &pc);
	}

	void CommandBuffer::pushConstant(const VkPipelineLayout layout, VkShaderStageFlags stageFlags, const glm::mat4& mvp) {
		
		vkCmdPushConstants(mCommandBuffer, layout, stageFlags, 0, sizeof(glm::mat4), &mvp);
	}

	void CommandBuffer::dispatch(uint32_t x, uint32_t y, uint32_t z) {
		
		vkCmdDispatch(mCommandBuffer, x, y, z);
	}

	void CommandBuffer::bindVertexBuffer(const std::vector<VkBuffer>& buffers) {
		
		std::vector<VkDeviceSize> offsets(buffers.size(), 0);

		vkCmdBindVertexBuffers(mCommandBuffer, 0, static_cast<uint32_t>(buffers.size()), buffers.data(), offsets.data());
	}

	void CommandBuffer::bindIndexBuffer(const VkBuffer& buffer) {
		
		vkCmdBindIndexBuffer(mCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void CommandBuffer::bindDescriptorSet(const VkPipelineLayout layout, const VkDescriptorSet& descriptorSet, uint32_t setIndex) {

		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, setIndex, 1, &descriptorSet, 0, nullptr);
	}

	void CommandBuffer::draw(size_t vertexCount){
	
		vkCmdDraw(mCommandBuffer, vertexCount, 1, 0, 0);
	}

	void CommandBuffer::drawIndex(size_t indexCount) {

		vkCmdDrawIndexed(mCommandBuffer, indexCount, 1, 0, 0, 0);
	}

	void CommandBuffer::drawIndexInstanced(uint32_t indexCount, uint32_t instancingCount) {
		
		vkCmdDrawIndexed(mCommandBuffer, indexCount, instancingCount, 0, 0, 0);
	}

	void CommandBuffer::drawIndexedIndirect(VkBuffer indirectBuffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
		
		vkCmdDrawIndexedIndirect(mCommandBuffer, indirectBuffer, offset, drawCount, stride);
	}

	void CommandBuffer::endRenderPass(){
	
		vkCmdEndRenderPass(mCommandBuffer);
	}

	void CommandBuffer::end(){
	
		if (vkEndCommandBuffer(mCommandBuffer) != VK_SUCCESS) {

			throw std::runtime_error("Error: failed to end command buffer");
		}
	}

	void CommandBuffer::copyBufferToBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t copyInfoCount, const std::vector<VkBufferCopy>& copyInfos) {
		
		vkCmdCopyBuffer(mCommandBuffer, srcBuffer, dstBuffer, copyInfoCount, copyInfos.data());
	}

	void CommandBuffer::copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t width, uint32_t height, uint32_t arrayLayer) {

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = arrayLayer;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(mCommandBuffer, srcBuffer, dstImage, dstImageLayout, 1, &region);

	}

	void CommandBuffer::submitSync(VkQueue queue, VkFence fence) {
		
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mCommandBuffer;

		vkQueueSubmit(queue, 1, &submitInfo, fence);

		vkQueueWaitIdle(queue);
	}

	void CommandBuffer::transferImageLayout(const VkImageMemoryBarrier &imageMemoryBarrier, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask) {

		vkCmdPipelineBarrier(mCommandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	}
}