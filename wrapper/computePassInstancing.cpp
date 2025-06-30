#include "computePassInstancing.h"

namespace LZ::Wrapper {

	ComputePassInstancing::ComputePassInstancing(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool) {
	
		mDevice = device;
		mCommandPool = commandPool;
	}

	void ComputePassInstancing::init(const std::string& fileName){
	
		mComputeShader = Wrapper::Shader::create(mDevice, fileName, VK_SHADER_STAGE_COMPUTE_BIT, "main");
		mComputePipeline = Wrapper::ComputePipeline::create(mDevice);
		mComputePipeline->setShader(mComputeShader);
		mComputePipeline->build();

	}

	void ComputePassInstancing::dispatch(const Wrapper::CommandPool::Ptr& commandPool, const LZ::Core::PushConstants& pc, uint32_t instanceCount) {

		auto commandBuffer = Wrapper::CommandBuffer::create(mDevice, commandPool);

		commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		commandBuffer->bindComputePipeline(mComputePipeline->getPipeline());
		commandBuffer->pushConstants(mComputePipeline->getLayout(), VK_SHADER_STAGE_COMPUTE_BIT, pc);

		const uint32_t groupSize = 32;
		const uint32_t dispatchCount = pc.bufferMatrices ? pc.bufferMatrices / (sizeof(glm::mat4) * groupSize) : 1;
		commandBuffer->dispatch(dispatchCount, 1, 1);

		commandBuffer->end();
		commandBuffer->submitSync(mDevice->getComputeQueue(), VK_NULL_HANDLE);
	}

	ComputePassInstancing::~ComputePassInstancing() {}
}
