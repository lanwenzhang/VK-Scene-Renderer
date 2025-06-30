#pragma once

#include "../common.h"
#include "../renderer/uniform/pushConstantsManager.h"
#include "device.h"
#include "shader.h"
#include "computePipeline.h"
#include "commandPool.h"
#include "commandBuffer.h"

namespace LZ::Wrapper {

	class ComputePassInstancing {
	public:

		using Ptr = std::shared_ptr<ComputePassInstancing>;
		static Ptr create(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool) {

			return std::make_shared<ComputePassInstancing>(device, commandPool);
		}

		ComputePassInstancing(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool);
		~ComputePassInstancing();

		void init(const std::string& fileName);
		void dispatch(const Wrapper::CommandPool::Ptr& commandPool, const LZ::Core::PushConstants& pc, uint32_t instanceCount);

	private:
		Wrapper::Device::Ptr mDevice{ nullptr };
		Wrapper::Shader::Ptr mComputeShader{ nullptr };
		Wrapper::ComputePipeline::Ptr mComputePipeline{ nullptr };
		Wrapper::CommandPool::Ptr mCommandPool{ nullptr };
		Wrapper::CommandBuffer::Ptr mCommandBuffer{ nullptr };
	};

}