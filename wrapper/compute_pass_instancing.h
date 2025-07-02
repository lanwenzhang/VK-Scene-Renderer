#pragma once

#include "../common.h"
#include "../renderer/uniform/push_constants_manager.h"
#include "device.h"
#include "shader.h"
#include "compute_pipeline.h"
#include "command_pool.h"
#include "command_buffer.h"

namespace lzvk::wrapper {

	class ComputePassInstancing {
	public:

		using Ptr = std::shared_ptr<ComputePassInstancing>;
		static Ptr create(const Device::Ptr& device, const CommandPool::Ptr& commandPool) {

			return std::make_shared<ComputePassInstancing>(device, commandPool);
		}

		ComputePassInstancing(const Device::Ptr& device, const CommandPool::Ptr& commandPool);
		~ComputePassInstancing();

		void init(const std::string& fileName);
		void dispatch(const CommandPool::Ptr& commandPool, const lzvk::core::PushConstants& pc, uint32_t instanceCount);

	private:
		Device::Ptr mDevice{ nullptr };
		Shader::Ptr mComputeShader{ nullptr };
		ComputePipeline::Ptr mComputePipeline{ nullptr };
		CommandPool::Ptr mCommandPool{ nullptr };
		CommandBuffer::Ptr mCommandBuffer{ nullptr };
	};

}