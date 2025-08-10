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
#include "../wrapper/framebuffer.h"
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
#include "../renderer/uniform/shadow_uniform_manager.h"
#include "../renderer/uniform/scene_texture_manager.h"
#include "../renderer/uniform/draw_data_uniform_manager.h"
#include "../renderer/uniform/ssao_uniform_manager.h"
#include "../renderer/uniform/blur_uniform_manager.h"
#include "../renderer/uniform/combine_uniform_manager.h"

#include "../renderer/texture/texture.h"
#include "../renderer/texture/cube_map_texture.h"
#include "../renderer/camera/camera.h"
#include "../renderer/light/directional_light.h"

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

		// framebuffer
		void createEnvironmentMap();
		void createShadowMap();
		void createGeometryFramebuffer();
		void createSSAOResources();
		void createBlurImages();

		// scene buffer
		void createSceneBuffers();

		// descriptor
		void createDescriptorSets();

		// imgui
		void createImGuiDescriptorPool();
		void initImGui();
		void cleanUpImGui();

		// pipelines
		void applyCommonPipelineState(const lzvk::wrapper::Pipeline::Ptr& pipeline, bool enableDepthWrite, VkCullModeFlagBits cullMode, PipelineType type);
		void createShadowPipeline();
		void createSkyboxPipeline();
		void createSceneGraphPipeline();
		void createSSAOPipeline();
		void createBlurPipelines();
		void createCombinePipeline();

		// command buffers
		void createCommandBuffers();
		void recordShadowPass(const lzvk::wrapper::CommandBuffer::Ptr& cmd);
		void transitionGeometryImages(const lzvk::wrapper::CommandBuffer::Ptr& cmd);
		void recordGeometryPass(const lzvk::wrapper::CommandBuffer::Ptr& cmd);
		void recordSSAOPass(const lzvk::wrapper::CommandBuffer::Ptr& cmd);
		void recordBlurPass(const lzvk::wrapper::CommandBuffer::Ptr& cmd);
		void recordCombinePass(const lzvk::wrapper::CommandBuffer::Ptr& cmd, uint32_t imageIndex);
		void recordImGuiPass(const lzvk::wrapper::CommandBuffer::Ptr& cmd, uint32_t imageIndex);
		void recordCommandBuffer(uint32_t imageIndex);

		// sync
		void createSyncObjects();

	private:

		unsigned int mWidth{ 600 };
		unsigned int mHeight{ 400 };

		unsigned int mShadowMapRes{ 4096 };
		unsigned int mIrradianceMapRes{ 128 };

		int mCurrentFrame{ 0 };
		const int MAX_FRAMES_IN_FLIGHT{ 2 };
		int mBlurPassCount{ 2 };

		lzvk::core::Window::Ptr mWindow{ nullptr };
		lzvk::wrapper::Instance::Ptr mInstance{ nullptr };
		lzvk::wrapper::Device::Ptr mDevice{ nullptr };
		lzvk::wrapper::Surface::Ptr mSurface{ nullptr };
		lzvk::wrapper::CommandPool::Ptr mCommandPool{ nullptr };
		lzvk::wrapper::SwapChain::Ptr mSwapChain{ nullptr };

		// pre compute
		lzvk::renderer::CubeMapTexture::Ptr mSkyboxCube{ nullptr };
		lzvk::wrapper::Image::Ptr mImage_IrradianceCube{ nullptr };
		lzvk::renderer::CubeMapTexture::Ptr mIrradianceCube{ nullptr };

		// pass 00 shadow
		lzvk::wrapper::Framebuffer::Ptr mFramebuffer_Shadow{ nullptr };
		lzvk::wrapper::Image::Ptr mImage_Shadow{ nullptr };
		lzvk::renderer::Texture::Ptr mTexture_Shadow{ nullptr };
		lzvk::wrapper::Pipeline::Ptr mShadowPipeline{ nullptr };


		// pass 01 geometry
		lzvk::wrapper::Framebuffer::Ptr mFramebuffer_Geometry{ nullptr };
		lzvk::wrapper::Image::Ptr mColorImage_Geometry{ nullptr };
		lzvk::wrapper::Image::Ptr mDepthImage_Geometry{ nullptr };

		lzvk::wrapper::Pipeline::Ptr mSkyboxPipeline{ nullptr };
		lzvk::wrapper::Pipeline::Ptr mSceneGraphPipeline{ nullptr };

		lzvk::renderer::SceneMeshRenderer::Ptr mSceneMesh{ nullptr };
		lzvk::renderer::FrameUniformManager::Ptr mFrameUniformManager{ nullptr };
		lzvk::renderer::ShadowUniformManager::Ptr mShadowUniformManager{ nullptr };
		lzvk::renderer::SkyboxUniformManager::Ptr mSkyboxUniformManager{ nullptr };
		lzvk::renderer::SSAOUniformManager::Ptr mSSAOUniformManager{ nullptr };
		lzvk::renderer::BlurUniformManager::Ptr mBlurUniformManager{ nullptr };
		lzvk::renderer::CombineUniformManager::Ptr mCombineUniformManager{ nullptr };

		// pass 02 ssao
		lzvk::renderer::Texture::Ptr mDepthTexture_SSAO{ nullptr };
		lzvk::renderer::Texture::Ptr mRotationTexture{ nullptr };
		lzvk::wrapper::Image::Ptr mAOImage_SSAO{ nullptr };
		lzvk::wrapper::ComputePipeline::Ptr mSSAOPipeline{ nullptr };

		// pass 03 blur
		lzvk::wrapper::Image::Ptr mAOImage_BlurPing{ nullptr };
		lzvk::wrapper::Image::Ptr mAOImage_BlurPong{ nullptr };
		lzvk::wrapper::Image::Ptr mFinalAOImage{ nullptr };

		lzvk::renderer::Texture::Ptr mDepthTexture_Blur{ nullptr };
		lzvk::renderer::Texture::Ptr mAOTexture_SSAO{ nullptr };
		lzvk::renderer::Texture::Ptr mAOTexture_BlurPing{ nullptr };
		lzvk::renderer::Texture::Ptr mAOTexture_BlurPong{ nullptr };

		lzvk::wrapper::ComputePipeline::Ptr mBlurPipelineH{ nullptr };
		lzvk::wrapper::ComputePipeline::Ptr mBlurPipelineV{ nullptr };

		// pass 04 combine
		lzvk::renderer::Texture::Ptr mColorTexture_Combine{ nullptr };
		lzvk::renderer::Texture::Ptr mAOTexture_Combine{ nullptr };
		lzvk::wrapper::Pipeline::Ptr mCombinePipeline{ nullptr };

		// vp matrix uniform
		lzvk::wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Frame{ nullptr };
		lzvk::wrapper::DescriptorPool::Ptr      mDescriptorPool_Frame{ nullptr };
		lzvk::wrapper::DescriptorSet::Ptr       mDescriptorSet_Frame{ nullptr };

		// shadow uniform
		lzvk::wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Shadow{ nullptr };
		lzvk::wrapper::DescriptorPool::Ptr      mDescriptorPool_Shadow{ nullptr };
		lzvk::wrapper::DescriptorSet::Ptr       mDescriptorSet_Shadow{ nullptr };

		// sky box uniform
		lzvk::wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Skybox{ nullptr };
		lzvk::wrapper::DescriptorPool::Ptr      mDescriptorPool_Skybox{ nullptr };
		lzvk::wrapper::DescriptorSet::Ptr       mDescriptorSet_Skybox{ nullptr };

		// ssao uniform
		lzvk::wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_SSAO{ nullptr };
		lzvk::wrapper::DescriptorPool::Ptr      mDescriptorPool_SSAO{ nullptr };
		lzvk::wrapper::DescriptorSet::Ptr       mDescriptorSet_SSAO{ nullptr };

		// blur uniform
		lzvk::wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Blur{ nullptr };
		lzvk::wrapper::DescriptorPool::Ptr      mDescriptorPool_Blur{ nullptr };
		std::vector<lzvk::wrapper::DescriptorSet::Ptr>       mDescriptorSet_BlurH{};
		std::vector<lzvk::wrapper::DescriptorSet::Ptr>       mDescriptorSet_BlurV{};

		// combine uniform
		lzvk::wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout_Combine{ nullptr };
		lzvk::wrapper::DescriptorPool::Ptr      mDescriptorPool_Combine{ nullptr };
		lzvk::wrapper::DescriptorSet::Ptr       mDescriptorSet_Combine{ nullptr };

		// imgui
		VkDescriptorPool mImGuiDescriptorPool = VK_NULL_HANDLE;

		// scene
		lzvk::loader::Scene    mSceneExterior;
		lzvk::loader::MeshData mMeshDataExterior;

		lzvk::loader::Scene    mSceneInterior;
		lzvk::loader::MeshData mMeshDataInterior;

		lzvk::loader::Scene    mScene;
		lzvk::loader::MeshData mMeshData;

		lzvk::renderer::Camera mCamera;
		VPMatrices mVPMatrices;

		lzvk::renderer::DirectionalLight mLight;
		VPMatrices mLightVP;
		float mLightTheta{ 90.0f };
		float mLightPhi{ -26.0f };


		std::vector<lzvk::wrapper::CommandBuffer::Ptr> mCommandBuffers{};
		std::vector<lzvk::wrapper::Semaphore::Ptr> mImageAvailableSemaphores{};
		std::vector<lzvk::wrapper::Semaphore::Ptr> mRenderFinishedSemaphores{};
		std::vector<lzvk::wrapper::Fence::Ptr> mInFlightFences{};
	};
}