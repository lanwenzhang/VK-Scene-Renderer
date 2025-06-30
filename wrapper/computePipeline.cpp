#include "computePipeline.h"

namespace LZ::Wrapper {

	ComputePipeline::ComputePipeline(const Device::Ptr& device){
		
		mDevice = device;
	}

	void ComputePipeline::setShader(const Shader::Ptr& computeShader){
		
		mComputeShader = computeShader;
	}
	void ComputePipeline::build(){
	
		if (!mComputeShader) {
			throw std::runtime_error("Compute shader is not set");
		}

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(glm::mat4) + sizeof(uint64_t) * 2 + sizeof(float) + sizeof(uint32_t);
		pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = 0;
		layoutInfo.pSetLayouts = nullptr;
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(mDevice->getDevice(), &layoutInfo, nullptr, &mLayout) != VK_SUCCESS) {
			
			throw std::runtime_error("Failed to create compute pipeline layout");

		}

		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		shaderStageInfo.module = mComputeShader->getShaderModule();
		shaderStageInfo.pName = mComputeShader->getShaderEntryPoint().c_str();

		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.stage = shaderStageInfo;
		pipelineInfo.layout = mLayout;

		if (vkCreateComputePipelines(mDevice->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mComputePipeline) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create compute pipeline");
		}
	}

	ComputePipeline::~ComputePipeline() {
	
		if (mComputePipeline != VK_NULL_HANDLE) {

			vkDestroyPipeline(mDevice->getDevice(), mComputePipeline, nullptr);
		}

		if (mLayout != VK_NULL_HANDLE) {

			vkDestroyPipelineLayout(mDevice->getDevice(), mLayout, nullptr);
		}

	}
}

