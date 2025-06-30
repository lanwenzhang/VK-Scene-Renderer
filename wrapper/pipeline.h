#pragma once

#include "../common.h"
#include "device.h"
#include "shader.h"
#include "renderpass.h"
#include "vulkanConfig.h"

namespace LZ::Wrapper {

	class Pipeline {
	public:

		Pipeline(const Device::Ptr &device, const RenderPass::Ptr &renderPass);
		~Pipeline();

		using Ptr = std::shared_ptr<Pipeline>;
		static Ptr create(const Device::Ptr& device, const RenderPass::Ptr& renderPass) { return std::make_shared<Pipeline>(device, renderPass); }

		void setShaderGroup(const std::vector<Shader::Ptr>& shaderGroup);
		void setViewports(const std::vector<VkViewport>& viewports) { mViewports = viewports; }
		void setScissors(const std::vector<VkRect2D>& scissors) { mScissors = scissors; }
		void setDynamicStates(const std::vector<VkDynamicState>& dynamicStates);
		void pushBlendAttachments(const VkPipelineColorBlendAttachmentState& blendAttachment) {

			mBlendAttachmentStates.push_back(blendAttachment);

		}

		void build();

		[[nodiscard]] auto getPipeline() const { return mPipeline; }
		[[nodiscard]] auto getLayout() const { return mLayout; }

	public:

		VkPipelineVertexInputStateCreateInfo mVertexInputState{};
		VkPipelineInputAssemblyStateCreateInfo mAssemblyState{};
		VkPipelineViewportStateCreateInfo mViewportState{};
		VkPipelineRasterizationStateCreateInfo mRasterState{};
		VkPipelineMultisampleStateCreateInfo mSampleState{};
		std::vector<VkPipelineColorBlendAttachmentState> mBlendAttachmentStates{};
		VkPipelineColorBlendStateCreateInfo mBlendState{};
		VkPipelineDepthStencilStateCreateInfo mDepthStencilState{};
		VkPipelineLayoutCreateInfo mLayoutState{};

		VkPipelineDynamicStateCreateInfo mDynamicState{};
		std::vector<VkDynamicState> mDynamicStatesStorage{};

		std::vector<VkDescriptorSetLayout> mSetLayoutsStorage{};

	private:

		Device::Ptr mDevice{ nullptr };
		RenderPass::Ptr mRenderPass{ nullptr };
		VkPipeline mPipeline{ VK_NULL_HANDLE };
		VkPipelineLayout mLayout{ VK_NULL_HANDLE };
		
		std::vector<Shader::Ptr> mShaders{};
		std::vector<VkViewport> mViewports{};
		std::vector<VkRect2D> mScissors{};
	};
}