#pragma once

#include "../base.h"
#include "device.h"
#include "shader.h"

namespace FF::Wrapper {

	class ComputePipeline {
	public:
		using Ptr = std::shared_ptr<ComputePipeline>;
		static Ptr create(const Device::Ptr& device) { return std::make_shared<ComputePipeline>(device); }

		ComputePipeline(const Device::Ptr& device);
		~ComputePipeline();

		void setShader(const Shader::Ptr& computeShader);
		void build();

		[[nodiscard]] auto getPipeline() const { return mComputePipeline; }
		[[nodiscard]] auto getLayout() const { return mLayout; }

	private:

		Device::Ptr mDevice{ nullptr };
		Shader::Ptr mComputeShader{ nullptr };
		VkPipeline  mComputePipeline{ VK_NULL_HANDLE };
		VkPipelineLayout mLayout{ VK_NULL_HANDLE };
	};
}

