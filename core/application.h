#pragma once

#include "window.h"
#include "type.h"
#include "../common.h"
#include "../wrapper/instance.h"
#include "../wrapper/device.h"
#include "../wrapper/Surface.h"
#include "../wrapper/swap_chain.h"

#include "../wrapper/shader.h"
#include "../wrapper/pipeline.h"
#include "../wrapper/render_pass.h"
#include "../wrapper/command_pool.h"
#include "../wrapper/command_buffer.h"
#include "../wrapper/semaphore.h"
#include "../wrapper/fence.h"
#include "../wrapper/buffer.h"
#include "../wrapper/descriptor_set_layout.h"
#include "../wrapper/descriptor_pool.h"
#include "../wrapper/description.h"
#include "../wrapper/descriptor_set.h"
#include "../wrapper/image.h"
#include "../wrapper/compute_pipeline.h"
#include "../wrapper/compute_pass_instancing.h"

#include "../loader/scene.h"
#include "../loader/mesh.h"

#include "../renderer/scene/scene_mesh_renderer.h"
#include "../renderer/uniform/frame_uniform_manager.h"
#include "../renderer/uniform/transform_uniform_manager.h"
#include "../renderer/uniform/material_uniform_manager.h"
#include "../renderer/uniform/skybox_uniform_manager.h"
#include "../renderer/uniform/scene_texture_manager.h"
#include "../renderer/uniform/draw_data_uniform_manager.h"
#include "../renderer/texture/texture.h"
#include "../renderer/texture/cube_map_texture.h"
#include "../renderer/camera/camera.h"

namespace lzvk::core{

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
		void applyCommonPipelineState(const lzvk::wrapper::Pipeline::Ptr& pipeline, bool enableDepthWrite, VkCullModeFlagBits cullMode, bool isSceneGraph);
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

		lzvk::core::Window::Ptr mWindow{ nullptr };
		lzvk::wrapper::Instance::Ptr mInstance{ nullptr };
		lzvk::wrapper::Device::Ptr mDevice{ nullptr };
		lzvk::wrapper::Surface::Ptr mSurface{ nullptr };
		lzvk::wrapper::CommandPool::Ptr mCommandPool{ nullptr };
		lzvk::wrapper::SwapChain::Ptr mSwapChain{ nullptr };

		lzvk::wrapper::RenderPass::Ptr mRenderPass{ nullptr };
		lzvk::wrapper::Pipeline::Ptr mSkyboxPipeline{ nullptr };
		lzvk::wrapper::Pipeline::Ptr mSceneGraphPipeline{ nullptr };

		std::vector<lzvk::wrapper::CommandBuffer::Ptr> mCommandBuffers{};
		std::vector<lzvk::wrapper::Semaphore::Ptr> mImageAvailableSemaphores{};
		std::vector<lzvk::wrapper::Semaphore::Ptr> mRenderFinishedSemaphores{};
		std::vector<lzvk::wrapper::Fence::Ptr> mInFlightFences{};
		
		lzvk::renderer::SceneMeshRenderer::Ptr mSceneMesh{ nullptr };
		lzvk::renderer::FrameUniformManager::Ptr mFrameUniformManager{ nullptr };
		lzvk::renderer::SkyboxUniformManager::Ptr mSkyboxUniformManager{ nullptr };

		// Frame Uniform (per-frame)
		lzvk::wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Frame{ nullptr };
		lzvk::wrapper::DescriptorPool::Ptr      mDescriptorPool_Frame{ nullptr };
		lzvk::wrapper::DescriptorSet::Ptr       mDescriptorSet_Frame{ nullptr };

		// Skybox 
		lzvk::wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Skybox{ nullptr };
		lzvk::wrapper::DescriptorPool::Ptr      mDescriptorPool_Skybox{ nullptr };
		lzvk::wrapper::DescriptorSet::Ptr       mDescriptorSet_Skybox{ nullptr };

		lzvk::loader::Scene    mSceneExterior;
		lzvk::loader::MeshData mMeshDataExterior;

		lzvk::loader::Scene    mSceneInterior;
		lzvk::loader::MeshData mMeshDataInterior;

		lzvk::loader::Scene    mScene;
		lzvk::loader::MeshData mMeshData;

		lzvk::renderer::Camera mCamera;
		VPMatrices mVPMatrices;
	};
}