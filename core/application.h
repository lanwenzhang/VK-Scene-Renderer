#pragma once

#include "window.h"
#include "type.h"
#include "../common.h"
#include "../wrapper/instance.h"
#include "../wrapper/device.h"
#include "../wrapper/windowSurface.h"
#include "../wrapper/swapChain.h"

#include "../wrapper/shader.h"
#include "../wrapper/pipeline.h"
#include "../wrapper/renderPass.h"
#include "../wrapper/commandPool.h"
#include "../wrapper/commandBuffer.h"
#include "../wrapper/semaphore.h"
#include "../wrapper/fence.h"
#include "../wrapper/buffer.h"
#include "../wrapper/descriptorSetLayout.h"
#include "../wrapper/descriptorPool.h"
#include "../wrapper/description.h"
#include "../wrapper/descriptorSet.h"
#include "../wrapper/image.h"
#include "../wrapper/computePipeline.h"
#include "../wrapper/computePassInstancing.h"

#include "../loader/scene.h"
#include "../loader/mesh.h"

#include "../renderer/scene/sceneMeshRenderer.h"
#include "../renderer/uniform/frameUniformManager.h"
#include "../renderer/uniform/transformUniformManager.h"
#include "../renderer/uniform/materialUniformManager.h"
#include "../renderer/uniform/skyboxUniformManager.h"
#include "../renderer/uniform/sceneTextureManager.h"
#include "../renderer/uniform/drawDataUniformManager.h"
#include "../renderer/texture/texture.h"
#include "../renderer/texture/cubemapTexture.h"
#include "../renderer/camera/camera.h"

namespace LZ::Core{

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
		void recreateSwapChain();
		void cleanupSwapChain();
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

	private:

		unsigned int mWidth{ 800 };
		unsigned int mHeight{ 600 };

		int mCurrentFrame{ 0 };
		const int MAX_FRAMES_IN_FLIGHT{ 2 };

		LZ::Core::Window::Ptr mWindow{ nullptr };
		LZ::Wrapper::Instance::Ptr mInstance{ nullptr };
		LZ::Wrapper::Device::Ptr mDevice{ nullptr };
		LZ::Wrapper::WindowSurface::Ptr mSurface{ nullptr };
		LZ::Wrapper::CommandPool::Ptr mCommandPool{ nullptr };
		LZ::Wrapper::SwapChain::Ptr mSwapChain{ nullptr };

		LZ::Wrapper::RenderPass::Ptr mRenderPass{ nullptr };
		LZ::Wrapper::Pipeline::Ptr mSkyboxPipeline{ nullptr };
		LZ::Wrapper::Pipeline::Ptr mSceneGraphPipeline{ nullptr };

		std::vector<LZ::Wrapper::CommandBuffer::Ptr> mCommandBuffers{};
		std::vector<LZ::Wrapper::Semaphore::Ptr> mImageAvailableSemaphores{};
		std::vector<LZ::Wrapper::Semaphore::Ptr> mRenderFinishedSemaphores{};
		std::vector<LZ::Wrapper::Fence::Ptr> mInFlightFences{};
		
		LZ::Renderer::SceneMeshRenderer::Ptr mSceneMesh{ nullptr };
		LZ::Renderer::FrameUniformManager::Ptr mFrameUniformManager{ nullptr };
		LZ::Renderer::TransformUniformManager::Ptr mTransformUniformManager{ nullptr };
		LZ::Renderer::MaterialUniformManager::Ptr mMaterialUniformManager{ nullptr };
		LZ::Renderer::SkyboxUniformManager::Ptr mSkyboxUniformManager{ nullptr };
		LZ::Renderer::DrawDataUniformManager::Ptr mDrawDataUniformManager{ nullptr };
		LZ::Renderer::SceneTextureManager::Ptr mSceneTextureManager{ nullptr };

		// Frame Uniform (per-frame)
		LZ::Wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Frame{ nullptr };
		LZ::Wrapper::DescriptorPool::Ptr      mDescriptorPool_Frame{ nullptr };
		LZ::Wrapper::DescriptorSet::Ptr       mDescriptorSet_Frame{ nullptr };

		// Static Uniforms (static resources)
		LZ::Wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Static{ nullptr };
		LZ::Wrapper::DescriptorPool::Ptr      mDescriptorPool_Static{ nullptr };
		LZ::Wrapper::DescriptorSet::Ptr       mDescriptorSet_Static{ nullptr };

		// Static Uniforms (diffuse texture)
		LZ::Wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Diffuse{ nullptr };
		LZ::Wrapper::DescriptorPool::Ptr      mDescriptorPool_Diffuse{ nullptr };
		LZ::Wrapper::DescriptorSet::Ptr       mDescriptorSet_Diffuse{ nullptr };

		LZ::Loader::Scene    mScene;
		LZ::Loader::MeshData mMeshData;
		LZ::Renderer::Camera mCamera;
		VPMatrices mVPMatrices;
	};
}