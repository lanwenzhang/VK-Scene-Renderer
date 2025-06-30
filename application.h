#pragma once

#include "base.h"
#include "vulkanWrapper/instance.h"
#include "vulkanWrapper/device.h"
#include "vulkanWrapper/window.h"
#include "vulkanWrapper/windowSurface.h"
#include "vulkanWrapper/swapChain.h"

#include "vulkanWrapper/shader.h"
#include "vulkanWrapper/pipeline.h"
#include "vulkanWrapper/renderPass.h"
#include "vulkanWrapper/commandPool.h"
#include "vulkanWrapper/commandBuffer.h"
#include "vulkanWrapper/semaphore.h"
#include "vulkanWrapper/fence.h"
#include "vulkanWrapper/buffer.h"
#include "vulkanWrapper/descriptorSetLayout.h"
#include "vulkanWrapper/descriptorPool.h"
#include "vulkanWrapper/description.h"
#include "vulkanWrapper/descriptorSet.h"
#include "vulkanWrapper/image.h"
#include "vulkanWrapper/computePipeline.h"
#include "vulkanWrapper/computePassInstancing.h"

#include "uniform/frameUniformManager.h"
#include "uniform/transformUniformManager.h"
#include "uniform/materialUniformManager.h"
#include "uniform/skyboxUniformManager.h"
#include "uniform/sceneTextureManager.h"
#include "uniform/drawDataUniformManager.h"

#include "texture/texture.h"
#include "texture/cubemapTexture.h"

#include "model.h"
#include "indirectModel.h"

#include "scene/scene.h"
#include "scene/mesh.h"
#include "scene/sceneMeshRenderer.h"

#include "camera.h"

namespace FF {

	class Application :public std::enable_shared_from_this<Application> {
	public:

		Application() = default;
		~Application() = default;

		void run();

		void onMouseMove(double xpos, double ypos);
		void onKeyDown(CAMERA_MOVE moveDirection);
		void enableMouseControl(bool enable);

	private:

		void initWindow();
		void initVulkan();
		void mainLoop();
		void render();
		void cleanUp();

	private:

		void createRenderPass();
		void createDescriptorSets();
		void applyCommonPipelineState(const Wrapper::Pipeline::Ptr& pipeline, bool enableDepthWrite, VkCullModeFlagBits cullMode, bool isSceneGraph);
		void createSkyboxPipeline();
		void createSceneGraphPipeline();

		void createCommandBuffers();
		void recordCommandBuffer(uint32_t imageIndex);

		void createSyncObjects();

		void recreateSwapChain();
		void cleanupSwapChain();

	private:

		unsigned int mWidth{ 800 };
		unsigned int mHeight{ 600 };

	private:

		int mCurrentFrame{ 0 };
		const int MAX_FRAMES_IN_FLIGHT{ 2 };

		Wrapper::Window::Ptr mWindow{ nullptr };
		Wrapper::Instance::Ptr mInstance{ nullptr };
		Wrapper::Device::Ptr mDevice{ nullptr };
		Wrapper::WindowSurface::Ptr mSurface{ nullptr };
		Wrapper::CommandPool::Ptr mCommandPool{ nullptr };
		Wrapper::SwapChain::Ptr mSwapChain{ nullptr };

		Wrapper::RenderPass::Ptr mRenderPass{ nullptr };
		Wrapper::Pipeline::Ptr mSkyboxPipeline{ nullptr };
		Wrapper::Pipeline::Ptr mSceneGraphPipeline{ nullptr };

		std::vector<Wrapper::CommandBuffer::Ptr> mCommandBuffers{};
		std::vector<Wrapper::Semaphore::Ptr> mImageAvailableSemaphores{};
		std::vector<Wrapper::Semaphore::Ptr> mRenderFinishedSemaphores{};
		std::vector<Wrapper::Fence::Ptr> mInFlightFences{};
		
		SceneMeshRenderer::Ptr mSceneMesh{ nullptr };
		FrameUniformManager::Ptr mFrameUniformManager{ nullptr };
		TransformUniformManager::Ptr mTransformUniformManager{ nullptr };
		MaterialUniformManager::Ptr mMaterialUniformManager{ nullptr };
		SkyboxUniformManager::Ptr mSkyboxUniformManager{ nullptr };
		DrawDataUniformManager::Ptr mDrawDataUniformManager{ nullptr };
		SceneTextureManager::Ptr mSceneTextureManager{ nullptr };

		// Frame Uniform (per-frame)
		Wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Frame{ nullptr };
		Wrapper::DescriptorPool::Ptr      mDescriptorPool_Frame{ nullptr };
		Wrapper::DescriptorSet::Ptr       mDescriptorSet_Frame{ nullptr };

		// Static Uniforms (static resources)
		Wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Static{ nullptr };
		Wrapper::DescriptorPool::Ptr      mDescriptorPool_Static{ nullptr };
		Wrapper::DescriptorSet::Ptr       mDescriptorSet_Static{ nullptr };

		// Textures descriptor set
		FF::Wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Diffuse{ nullptr };
		FF::Wrapper::DescriptorPool::Ptr      mDescriptorPool_Diffuse{ nullptr };
		FF::Wrapper::DescriptorSet::Ptr       mDescriptorSet_Diffuse{ nullptr };

		//FF::Wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Emissive{ nullptr };
		//FF::Wrapper::DescriptorPool::Ptr      mDescriptorPool_Emissive{ nullptr };
		//FF::Wrapper::DescriptorSet::Ptr       mDescriptorSet_Emissive{ nullptr };

		//FF::Wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Occlusion{ nullptr };
		//FF::Wrapper::DescriptorPool::Ptr      mDescriptorPool_Occlusion{ nullptr };
		//FF::Wrapper::DescriptorSet::Ptr       mDescriptorSet_Occlusion{ nullptr };

		Scene mScene;
		MeshData mMeshData;
		VPMatrices	mVPMatrices;
		Camera      mCamera;
	};
}