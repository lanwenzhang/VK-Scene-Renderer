#include "application.h"
#include "../tools/scene_tools.h"
#include "../imgui/imgui.h"                     
#include "../imgui/imgui_impl_vulkan.h"
#include "../imgui/imgui_impl_glfw.h"

namespace lzvk::core {

	void Application::run() {

		initWindow();
		initVulkan();
		mainLoop();
		cleanUp();
	}

	void Application::onMouseMove(double xpos, double ypos) {
		
		if (!ImGui::GetIO().WantCaptureMouse) {
			mCamera.onMouseMove(xpos, ypos);
		}
	}

	void Application::onKeyDown(CAMERA_MOVE moveDirection) {
		
		mCamera.move(moveDirection);
	}

	void Application::enableMouseControl(bool enable) {
		
		mCamera.setMouseControl(enable);
	}

	void Application::initWindow() {

		mWindow = Window::create(mWidth, mHeight);
		mWindow->setApp(shared_from_this());

		mCamera.setPosition(glm::vec3(0.0f, 10.0f, 3.0f));
		mCamera.lookAt(glm::vec3(0.0f, 10.0f, 3.0f), glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		mCamera.update();
		mCamera.setPerpective(60.0f, (float)mWidth / (float)mHeight, 0.1f, 200.0f);
		mCamera.setSpeed(0.01f);

		mLight.setDirectionFromSpherical(mLightTheta, mLightPhi);
	}

	void Application::initVulkan() {

		// 1 Set up environment
		mInstance = lzvk::wrapper::Instance::create(true);
		mSurface = lzvk::wrapper::Surface::create(mInstance, mWindow);
		mDevice = lzvk::wrapper::Device::create(mInstance, mSurface);
		mCommandPool = lzvk::wrapper::CommandPool::create(mDevice);
		mSwapChain = lzvk::wrapper::SwapChain::create(mDevice, mWindow, mSurface, mCommandPool);
		mWidth = mSwapChain->getExtent().width;
		mHeight = mSwapChain->getExtent().height;

		// 2 create resources and frame buffer
		// 2.1 pass 01 geometry
		createShadowMap();
		createGeometryFramebuffer();
		createSSAOResources();
		createBlurImages();

		// 3 create scene buffer
		createSceneBuffers();

		// 4 create descriptor
		createDescriptorSets();

		createImGuiDescriptorPool();
		initImGui();

		//  create skybox texture
		createEnvironmentMap();

		// 5 create pipeline
		createShadowPipeline();
		createSkyboxPipeline();
		createSceneGraphPipeline();
		createSSAOPipeline();
		createBlurPipelines();
		createCombinePipeline();

		// 6 create command buffers
		createCommandBuffers();

		// 7 create sync
		createSyncObjects();

	}

	void Application::createEnvironmentMap() {

		// 1 Cube map
		mSkyboxCube = lzvk::renderer::CubeMapTexture::create(mDevice, mCommandPool, "assets/skybox/immenstadter_horn_2k_prefilter.ktx");
		mIrradianceCube = lzvk::renderer::CubeMapTexture::create(mDevice, mCommandPool, "assets/skybox/immenstadter_horn_2k_irradiance.ktx");

		auto cmd2 = lzvk::wrapper::CommandBuffer::create(mDevice, mCommandPool);
		cmd2->begin();
		cmd2->transitionImageLayout(
			mSkyboxCube->getImage()->getImage(),
			mSkyboxCube->getImage()->getFormat(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 6
		);

		cmd2->transitionImageLayout(
			mIrradianceCube->getImage()->getImage(),
			mIrradianceCube->getImage()->getFormat(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 6
		);

		cmd2->end();
		cmd2->submitSync(mDevice->getGraphicQueue());
		
		// 5. generate irradiance texture
		mSkyboxUniformManager->updateCubeMap(mDescriptorSet_Skybox, mSkyboxCube, 0);
		mSkyboxUniformManager->updateIrradianceMap(mDescriptorSet_Skybox, mIrradianceCube, 0);
	}

	void Application::createShadowMap() {

		mImage_Shadow = lzvk::wrapper::Image::create(
			mDevice,
			mShadowMapRes, mShadowMapRes,
			lzvk::wrapper::Image::findDepthFormat(mDevice),
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT
		);

		mFramebuffer_Shadow = lzvk::wrapper::Framebuffer::create(mDevice, mShadowMapRes, mShadowMapRes, true);
		mFramebuffer_Shadow->addDepthAttachment(mImage_Shadow);
	}

	void Application::createGeometryFramebuffer() {
		
		// 1 color image
		mColorImage_Geometry = lzvk::wrapper::Image::create(
			mDevice,
			mWidth, mHeight,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT
		);



		// 2 depth image
		mDepthImage_Geometry = lzvk::wrapper::Image::create(
			mDevice,
			mWidth, mHeight,
			lzvk::wrapper::Image::findDepthFormat(mDevice),
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT
		);


		// 3 create framebuffer
		mFramebuffer_Geometry = lzvk::wrapper::Framebuffer::create(mDevice, mWidth, mHeight, true);
		mFramebuffer_Geometry->addColorAttachment(mColorImage_Geometry);
		mFramebuffer_Geometry->addDepthAttachment(mDepthImage_Geometry);
	}

	void Application::createSSAOResources() {

		mRotationTexture = lzvk::renderer::Texture::create(mDevice, mCommandPool, "assets/ssao/rot_texture.bmp");
		mAOImage_SSAO = lzvk::wrapper::Image::create(
			mDevice,
			mWidth, mHeight,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT
		);

	}

	void Application::createBlurImages() {

		mAOImage_BlurPing = lzvk::wrapper::Image::create(
			mDevice,
			mWidth, mHeight,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT
		);

		mAOImage_BlurPong = lzvk::wrapper::Image::create(
			mDevice,
			mWidth, mHeight,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT
		);

	}

	void Application::createSceneBuffers() {

		const std::string exteriorMeshCache = "assets/.cache/exterior.meshes";
		const std::string exteriorSceneCache = "assets/.cache/exterior.scene";
		const std::string interiorMeshCache = "assets/.cache/interior.meshes";
		const std::string interiorSceneCache = "assets/.cache/interior.scene";

		const std::string sourceModelExterior = "assets/bistro/Exterior/exterior.obj";
		const std::string sourceModelInterior = "assets/bistro/Interior/interior.obj";

		// ---------- Load EXTERIOR ----------
		bool exteriorCacheLoaded =
			lzvk::loader::loadMeshData(exteriorMeshCache, mMeshDataExterior) &&
			lzvk::loader::loadScene(exteriorSceneCache, mSceneExterior);

		if (!exteriorCacheLoaded) {
			printf("[Application] Cache not found for EXTERIOR. Loading from OBJ...\n");

			bool loaded = lzvk::loader::loadMeshFile(sourceModelExterior, mMeshDataExterior, mSceneExterior);
			if (!loaded) {
				printf("[Application] Failed to load EXTERIOR mesh file!\n");
				throw std::runtime_error("Failed to load EXTERIOR mesh file.");
			}

			// merge foliage
			lzvk::tools::mergeNodesWithMaterial(mSceneExterior, mMeshDataExterior, "Foliage_Linde_Tree_Large_Orange_Leaves");
			lzvk::tools::mergeNodesWithMaterial(mSceneExterior, mMeshDataExterior, "Foliage_Linde_Tree_Large_Green_Leaves");
			lzvk::tools::mergeNodesWithMaterial(mSceneExterior, mMeshDataExterior, "Foliage_Linde_Tree_Large_Trunk");

			// save cache
			lzvk::loader::saveMeshData(exteriorMeshCache, mMeshDataExterior);
			lzvk::loader::saveScene(exteriorSceneCache, mSceneExterior);

			printf("[Application] EXTERIOR loaded and cached.\n");
		}
		else {
			printf("[Application] Loaded EXTERIOR from cache.\n");
		}

		// ---------- Load INTERIOR ----------
		bool interiorCacheLoaded =
			lzvk::loader::loadMeshData(interiorMeshCache, mMeshDataInterior) &&
			lzvk::loader::loadScene(interiorSceneCache, mSceneInterior);

		if (!interiorCacheLoaded) {
			printf("[Application] Cache not found for INTERIOR. Loading from OBJ...\n");

			bool loaded = lzvk::loader::loadMeshFile(sourceModelInterior, mMeshDataInterior, mSceneInterior);
			if (!loaded) {
				printf("[Application] Failed to load INTERIOR mesh file!\n");
				throw std::runtime_error("Failed to load INTERIOR mesh file.");
			}

			// save cache
			lzvk::loader::saveMeshData(interiorMeshCache, mMeshDataInterior);
			lzvk::loader::saveScene(interiorSceneCache, mSceneInterior);

			printf("[Application] INTERIOR loaded and cached.\n");
		}
		else {
			printf("[Application] Loaded INTERIOR from cache.\n");
		}

		printf("[Application] EXTERIOR meshes = %zu, draw data = %zu, hierarchy = %zu\n ",
			mMeshDataExterior.meshes.size(), mSceneExterior.drawDataArray.size(), mSceneExterior.hierarchy.size());

		printf("[Application] INTERIOR meshes = %zu, draw data = %zu, hierarchy = %zu\n ",
			mMeshDataInterior.meshes.size(), mSceneInterior.drawDataArray.size(), mSceneInterior.hierarchy.size());

		// ---------- Merge SCENES ----------
		lzvk::tools::mergeScenes(
			mScene,
			{ &mSceneExterior, &mSceneInterior },
			{},
		{
			static_cast<uint32_t>(mMeshDataExterior.meshes.size()),
			static_cast<uint32_t>(mMeshDataInterior.meshes.size())
		}
		);

		printf("[Application] Merged scene hierarchy = %zu\n", mScene.hierarchy.size());
		printf("[Application] Merged scene drawData = %zu\n", mScene.drawDataArray.size());


		// ---------- Debug: MeshData before merge ----------
		printf("[Debug] Exterior: meshes = %zu, indices = %zu, vertexData = %zu bytes\n",
			mMeshDataExterior.meshes.size(),
			mMeshDataExterior.indexData.size(),
			mMeshDataExterior.vertexData.size());

		printf("[Debug] Interior: meshes = %zu, indices = %zu, vertexData = %zu bytes\n",
			mMeshDataInterior.meshes.size(),
			mMeshDataInterior.indexData.size(),
			mMeshDataInterior.vertexData.size());

		// ---------- Merge MESH DATA (test only) ----------
		lzvk::tools::mergeMeshData(mMeshData, mMeshDataExterior, mMeshDataInterior);
		lzvk::tools::mergeMaterialLists(mMeshData, { &mMeshDataExterior, &mMeshDataInterior });

		mScene.localTransform[0] = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));
		lzvk::loader::recalculateGlobalTransforms(mScene);

		mSceneMesh = lzvk::renderer::SceneMeshRenderer::create(mDevice, mCommandPool, mMeshData, mScene, MAX_FRAMES_IN_FLIGHT);

	}

	void Application::createDescriptorSets()
	{
		//
		// ========== Frame Uniform ==========
		//

		mFrameUniformManager = lzvk::renderer::FrameUniformManager::create();
		mFrameUniformManager->init(mDevice, MAX_FRAMES_IN_FLIGHT);

		std::vector<lzvk::wrapper::UniformParameter::Ptr> frameParams =
			mFrameUniformManager->getParams();

		mDescriptorSetLayout_Frame = lzvk::wrapper::DescriptorSetLayout::create(mDevice);
		mDescriptorSetLayout_Frame->build(frameParams);

		mDescriptorPool_Frame = lzvk::wrapper::DescriptorPool::create(mDevice);
		mDescriptorPool_Frame->build(frameParams, MAX_FRAMES_IN_FLIGHT);

		mDescriptorSet_Frame = lzvk::wrapper::DescriptorSet::create(
			mDevice,
			frameParams,
			mDescriptorSetLayout_Frame,
			mDescriptorPool_Frame,
			MAX_FRAMES_IN_FLIGHT
		);

		//
		// ========== Shadow Uniform ==========
		//

		mShadowUniformManager = lzvk::renderer::ShadowUniformManager::create();
		mShadowUniformManager->init(mDevice);

		auto shadowParams = mShadowUniformManager->getParams();

		mDescriptorSetLayout_Shadow = lzvk::wrapper::DescriptorSetLayout::create(mDevice);
		mDescriptorSetLayout_Shadow->build(shadowParams);

		mDescriptorPool_Shadow = lzvk::wrapper::DescriptorPool::create(mDevice);
		mDescriptorPool_Shadow->build(shadowParams, MAX_FRAMES_IN_FLIGHT);

		mDescriptorSet_Shadow = lzvk::wrapper::DescriptorSet::create(
			mDevice,
			shadowParams,
			mDescriptorSetLayout_Shadow,
			mDescriptorPool_Shadow,
			MAX_FRAMES_IN_FLIGHT
		);



		//
		// ========== Skybox Uniform ==========
		//

		mSkyboxUniformManager = lzvk::renderer::SkyboxUniformManager::create();
		mSkyboxUniformManager->init(mDevice);

		auto skyboxParams = mSkyboxUniformManager->getParams();

		mDescriptorSetLayout_Skybox = lzvk::wrapper::DescriptorSetLayout::create(mDevice);
		mDescriptorSetLayout_Skybox->build(skyboxParams);

		mDescriptorPool_Skybox = lzvk::wrapper::DescriptorPool::create(mDevice);
		mDescriptorPool_Skybox->build(skyboxParams, 1);

		mDescriptorSet_Skybox = lzvk::wrapper::DescriptorSet::create(
			mDevice,
			skyboxParams,
			mDescriptorSetLayout_Skybox,
			mDescriptorPool_Skybox,
			1
		);

		//
		// ========== SSAO Descriptor ==========
		//

		mSSAOUniformManager = lzvk::renderer::SSAOUniformManager::create();
		mSSAOUniformManager->init(mDevice);

		std::vector<lzvk::wrapper::UniformParameter::Ptr> ssaoParams = mSSAOUniformManager->getParams();

		mDescriptorSetLayout_SSAO = lzvk::wrapper::DescriptorSetLayout::create(mDevice);
		mDescriptorSetLayout_SSAO->build(ssaoParams);

		mDescriptorPool_SSAO = lzvk::wrapper::DescriptorPool::create(mDevice);
		mDescriptorPool_SSAO->build(ssaoParams, MAX_FRAMES_IN_FLIGHT);

		mDescriptorSet_SSAO = lzvk::wrapper::DescriptorSet::create(
			mDevice,
			ssaoParams,
			mDescriptorSetLayout_SSAO,
			mDescriptorPool_SSAO,
			MAX_FRAMES_IN_FLIGHT
		);

	//
	// ========== Blur Uniform ==========
	//

		mBlurUniformManager = lzvk::renderer::BlurUniformManager::create();
		mBlurUniformManager->init(mDevice);

		std::vector<lzvk::wrapper::UniformParameter::Ptr> blurParams = mBlurUniformManager->getParams();

		mDescriptorSetLayout_Blur = lzvk::wrapper::DescriptorSetLayout::create(mDevice);
		mDescriptorSetLayout_Blur->build(blurParams);

		mDescriptorPool_Blur = lzvk::wrapper::DescriptorPool::create(mDevice);
		mDescriptorPool_Blur->build(blurParams, mBlurPassCount * 2 * MAX_FRAMES_IN_FLIGHT);
		
		mDescriptorSet_BlurH.reserve(mBlurPassCount);
		mDescriptorSet_BlurV.reserve(mBlurPassCount);

		for (int i = 0; i < mBlurPassCount; i++) {

			auto setBlurH = lzvk::wrapper::DescriptorSet::create(
				mDevice,
				blurParams,
				mDescriptorSetLayout_Blur,
				mDescriptorPool_Blur,
				MAX_FRAMES_IN_FLIGHT
			);

			mDescriptorSet_BlurH.push_back(setBlurH);

			auto setBlurV = lzvk::wrapper::DescriptorSet::create(
				mDevice,
				blurParams,
				mDescriptorSetLayout_Blur,
				mDescriptorPool_Blur,
				MAX_FRAMES_IN_FLIGHT
			);

			mDescriptorSet_BlurV.push_back(setBlurV);
		}


		//
		// ========== Combine Uniform ==========
		//
		mCombineUniformManager = lzvk::renderer::CombineUniformManager::create();
		mCombineUniformManager->init(mDevice);

		auto combineParams = mCombineUniformManager->getParams();

		mDescriptorSetLayout_Combine = lzvk::wrapper::DescriptorSetLayout::create(mDevice);
		mDescriptorSetLayout_Combine->build(combineParams);

		mDescriptorPool_Combine = lzvk::wrapper::DescriptorPool::create(mDevice);
		mDescriptorPool_Combine->build(combineParams, MAX_FRAMES_IN_FLIGHT);

		mDescriptorSet_Combine = lzvk::wrapper::DescriptorSet::create(
			mDevice, combineParams,
			mDescriptorSetLayout_Combine,
			mDescriptorPool_Combine,
			MAX_FRAMES_IN_FLIGHT);
	}

	void Application::createImGuiDescriptorPool() {
		VkDescriptorPoolSize pool_sizes[] = {
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		};

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = 1;
		pool_info.pPoolSizes = pool_sizes;

		if (vkCreateDescriptorPool(mDevice->getDevice(), &pool_info, nullptr, &mImGuiDescriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create ImGui descriptor pool");
		}
	}

	void Application::initImGui() {

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImGui::StyleColorsDark();

		// 2. Platform + Renderer bindings
		ImGui_ImplGlfw_InitForVulkan(mWindow->getWindow(), true);

		// 3. Init info
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = mInstance->getInstance();
		init_info.PhysicalDevice = mDevice->getPhysicalDevice();
		init_info.Device = mDevice->getDevice();
		init_info.QueueFamily = mDevice->getGraphicQueueFamily().value(); 
		init_info.Queue = mDevice->getGraphicQueue();
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = mImGuiDescriptorPool;
		init_info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
		init_info.ImageCount = static_cast<uint32_t>(mSwapChain->getImageCount());
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.UseDynamicRendering = true;
		init_info.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
		VkFormat color_format = mSwapChain->getFormat();
		init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &color_format;
		init_info.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;

		ImGui_ImplVulkan_Init(&init_info);
	}

	void Application::cleanUpImGui() {

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		vkDestroyDescriptorPool(mDevice->getDevice(), mImGuiDescriptorPool, nullptr);

	}

	void Application::applyCommonPipelineState(const lzvk::wrapper::Pipeline::Ptr& pipeline, bool enableDepthWrite, VkCullModeFlagBits cullMode, PipelineType type) {
		
		// Dynamic state
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		if (type == PipelineType::Shadow) {

			dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
		}
		pipeline->setDynamicStates(dynamicStates);

		// Rasterization
		pipeline->mRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipeline->mRasterState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->mRasterState.lineWidth = 1.0f;
		pipeline->mRasterState.cullMode = cullMode;
		pipeline->mRasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		pipeline->mRasterState.depthBiasEnable = type == PipelineType::Shadow ? VK_TRUE : VK_FALSE;
		pipeline->mRasterState.depthBiasConstantFactor = 0.0f;
		pipeline->mRasterState.depthBiasClamp = 0.0f;
		pipeline->mRasterState.depthBiasSlopeFactor = 0.0f;

		// MSAA
		pipeline->mSampleState.sampleShadingEnable = VK_FALSE;
		pipeline->mSampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		pipeline->mSampleState.minSampleShading = 1.0f;
		pipeline->mSampleState.pSampleMask = nullptr;
		pipeline->mSampleState.alphaToCoverageEnable = VK_FALSE;
		pipeline->mSampleState.alphaToOneEnable = VK_FALSE;

		// Depth stencil
		pipeline->mDepthStencilState.depthTestEnable = VK_TRUE;
		pipeline->mDepthStencilState.depthWriteEnable = enableDepthWrite ? VK_TRUE : VK_FALSE;
		pipeline->mDepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;

		// Blend
		VkPipelineColorBlendAttachmentState blendAttachment{};
		blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		blendAttachment.blendEnable = VK_FALSE;
		blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

		blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		pipeline->pushBlendAttachments(blendAttachment);

		pipeline->mBlendState.logicOpEnable = VK_FALSE;
		pipeline->mBlendState.logicOp = VK_LOGIC_OP_COPY;
		pipeline->mBlendState.blendConstants[0] = 0.0f;
		pipeline->mBlendState.blendConstants[1] = 0.0f;
		pipeline->mBlendState.blendConstants[2] = 0.0f;
		pipeline->mBlendState.blendConstants[3] = 0.0f;

		switch (type)
		{
			case PipelineType::Irradiance: {

				pipeline->mSetLayoutsStorage = {
				mDescriptorSetLayout_Skybox->getLayout()
				};


				VkPushConstantRange irradiancePcRange{
					VK_SHADER_STAGE_VERTEX_BIT,
					0,
					sizeof(IrradiancePushConstant)
				};
				pipeline->mPushConstantRanges = { irradiancePcRange };
			}
								 break;

			case PipelineType::Shadow: {

				pipeline->mSetLayoutsStorage = {
				mDescriptorSetLayout_Frame->getLayout(),
				mSceneMesh->getDescriptorSetLayout_Static()->getLayout(),
				};
			}
								 break;
			case PipelineType::SceneGraph: {

				pipeline->mSetLayoutsStorage = {
					mDescriptorSetLayout_Frame->getLayout(),
					mSceneMesh->getDescriptorSetLayout_Static()->getLayout(),
					mSceneMesh->getDescriptorSetLayout_Diffuse()->getLayout(),
					mSceneMesh->getDescriptorSetLayout_Emissive()->getLayout(),
					mSceneMesh->getDescriptorSetLayout_Normal()->getLayout(),
					mSceneMesh->getDescriptorSetLayout_Opacity()->getLayout(),
					mSceneMesh->getDescriptorSetLayout_Specular()->getLayout(),
					mDescriptorSetLayout_Shadow->getLayout(),
					mDescriptorSetLayout_Skybox->getLayout()
				};

				VkPushConstantRange lightPcRange{
						VK_SHADER_STAGE_FRAGMENT_BIT,
						0,
						sizeof(LightPushConstant)
				};
				pipeline->mPushConstantRanges = { lightPcRange };

			}
									 break;
			case PipelineType::Skybox: {
				pipeline->mSetLayoutsStorage = {
					mDescriptorSetLayout_Frame->getLayout(),
					mDescriptorSetLayout_Skybox->getLayout()
				};
			}
								 break;

			case PipelineType::Combine: {
				
				pipeline->mSetLayoutsStorage = {
					mDescriptorSetLayout_Combine->getLayout()
				};

				static VkPushConstantRange combinePcRange{
					VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(float) * 2
				};
				pipeline->mPushConstantRanges = { combinePcRange };

			}
		
				break;
		}

		pipeline->mLayoutState.setLayoutCount = static_cast<uint32_t>(pipeline->mSetLayoutsStorage.size());
		pipeline->mLayoutState.pSetLayouts = pipeline->mSetLayoutsStorage.data();
		pipeline->mLayoutState.flags = 0;

		if (!pipeline->mPushConstantRanges.empty()) {
			pipeline->mLayoutState.pushConstantRangeCount = static_cast<uint32_t>(pipeline->mPushConstantRanges.size());
			pipeline->mLayoutState.pPushConstantRanges = pipeline->mPushConstantRanges.data();
		}
		else {
			pipeline->mLayoutState.pushConstantRangeCount = 0;
			pipeline->mLayoutState.pPushConstantRanges = nullptr;
		}

	}

	void Application::createShadowPipeline() {

		mShadowPipeline = lzvk::wrapper::Pipeline::create(mDevice);

		VkFormat depthFormat = mImage_Shadow->getFormat();
		mShadowPipeline->setDepthAttachmentFormat(depthFormat);

		if (depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT) {
			mShadowPipeline->setStencilAttachmentFormat(depthFormat);
		}

		// shaders
		std::vector<lzvk::wrapper::Shader::Ptr> shaderGroup{};
		shaderGroup.push_back(lzvk::wrapper::Shader::create(mDevice, "shaders/shadow/shadow_vs.spv", VK_SHADER_STAGE_VERTEX_BIT, "main"));
		shaderGroup.push_back(lzvk::wrapper::Shader::create(mDevice, "shaders/shadow/shadow_fs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main"));
		mShadowPipeline->setShaderGroup(shaderGroup);

		// vertex
		auto vertexBindingDes = mSceneMesh->getVertexInputBindingDescriptions();
		auto attributeDes = mSceneMesh->getAttributeDescriptions();
		mShadowPipeline->mVertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDes.size());
		mShadowPipeline->mVertexInputState.pVertexBindingDescriptions = vertexBindingDes.data();
		mShadowPipeline->mVertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDes.size());
		mShadowPipeline->mVertexInputState.pVertexAttributeDescriptions = attributeDes.data();

		// Input assembly
		mShadowPipeline->mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mShadowPipeline->mAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		mShadowPipeline->mAssemblyState.primitiveRestartEnable = VK_FALSE;

		applyCommonPipelineState(mShadowPipeline, true, VK_CULL_MODE_BACK_BIT, PipelineType::Shadow);

		mShadowPipeline->build();

	}

	void Application::createSkyboxPipeline() {

		mSkyboxPipeline = lzvk::wrapper::Pipeline::create(mDevice);
		mSkyboxPipeline->setColorAttachmentFormats({ mColorImage_Geometry->getFormat() });

		VkFormat depthFormat = mDepthImage_Geometry->getFormat();
		mSkyboxPipeline->setDepthAttachmentFormat(depthFormat);

		if (depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT) {
			mSkyboxPipeline->setStencilAttachmentFormat(depthFormat);
		}

		// Shaders
		std::vector<lzvk::wrapper::Shader::Ptr> shaderGroup{};
		shaderGroup.push_back(lzvk::wrapper::Shader::create(mDevice, "shaders/cubemap/cubemapvs.spv", VK_SHADER_STAGE_VERTEX_BIT, "main"));
		shaderGroup.push_back(lzvk::wrapper::Shader::create(mDevice, "shaders/cubemap/cubemapfs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main"));
		mSkyboxPipeline->setShaderGroup(shaderGroup);

		// No vertex buffer
		mSkyboxPipeline->mVertexInputState.vertexBindingDescriptionCount = 0;
		mSkyboxPipeline->mVertexInputState.pVertexBindingDescriptions = nullptr;
		mSkyboxPipeline->mVertexInputState.vertexAttributeDescriptionCount = 0;
		mSkyboxPipeline->mVertexInputState.pVertexAttributeDescriptions = nullptr;

		// Input assembly
		mSkyboxPipeline->mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mSkyboxPipeline->mAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		mSkyboxPipeline->mAssemblyState.primitiveRestartEnable = VK_FALSE;

		// Common states
		applyCommonPipelineState(mSkyboxPipeline, false, VK_CULL_MODE_FRONT_BIT, PipelineType::Skybox);

		// Render pass
		mSkyboxPipeline->build();
	}

	void Application::createSceneGraphPipeline() {

		mSceneGraphPipeline = lzvk::wrapper::Pipeline::create(mDevice);
		mSceneGraphPipeline->setColorAttachmentFormats({ mColorImage_Geometry->getFormat() });
		
		VkFormat depthFormat = mDepthImage_Geometry->getFormat();

		mSceneGraphPipeline->setDepthAttachmentFormat(depthFormat);
		if (depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT) {
			mSceneGraphPipeline->setStencilAttachmentFormat(depthFormat);
		}

		std::vector<lzvk::wrapper::Shader::Ptr> shaderGroup{};
		shaderGroup.push_back(lzvk::wrapper::Shader::create(mDevice, "shaders/scene_graph/scene_graph_vs.spv", VK_SHADER_STAGE_VERTEX_BIT, "main"));
		shaderGroup.push_back(lzvk::wrapper::Shader::create(mDevice, "shaders/scene_graph/scene_graph_fs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main"));
		mSceneGraphPipeline->setShaderGroup(shaderGroup);

		// Vertex input
		auto vertexBindingDes = mSceneMesh->getVertexInputBindingDescriptions();
		auto attributeDes = mSceneMesh->getAttributeDescriptions();
		mSceneGraphPipeline->mVertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDes.size());
		mSceneGraphPipeline->mVertexInputState.pVertexBindingDescriptions = vertexBindingDes.data();
		mSceneGraphPipeline->mVertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDes.size());
		mSceneGraphPipeline->mVertexInputState.pVertexAttributeDescriptions = attributeDes.data();

		mSceneGraphPipeline->mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mSceneGraphPipeline->mAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		mSceneGraphPipeline->mAssemblyState.primitiveRestartEnable = VK_FALSE;

		applyCommonPipelineState(mSceneGraphPipeline, true, VK_CULL_MODE_BACK_BIT, PipelineType::SceneGraph);

		mSceneGraphPipeline->build();
	}

	void Application::createSSAOPipeline() {

		mSSAOPipeline = lzvk::wrapper::ComputePipeline::create(mDevice);

		auto ssaoShader = lzvk::wrapper::Shader::create(mDevice, "shaders/ssao/ssao_comp.spv", VK_SHADER_STAGE_COMPUTE_BIT, "main");
		mSSAOPipeline->setShader(ssaoShader);
		mSSAOPipeline->setDescriptorSetLayouts({ mDescriptorSetLayout_SSAO->getLayout() });
		mSSAOPipeline->build();
	}

	void Application::createBlurPipelines() {

		// Load shader module (shared for both)
		auto blurShader = lzvk::wrapper::Shader::create(mDevice, "shaders/ssao/blur_comp.spv", VK_SHADER_STAGE_COMPUTE_BIT, "main");

		const uint32_t kSpecHorizontal = 1;
		const uint32_t kSpecVertical = 0;

		// Horizontal pipeline
		mBlurPipelineH = lzvk::wrapper::ComputePipeline::create(mDevice);
		mBlurPipelineH->setShader(blurShader);
		mBlurPipelineH->setDescriptorSetLayouts({ mDescriptorSetLayout_Blur->getLayout() });
		mBlurPipelineH->setSpecializationConstant(0, sizeof(uint32_t), &kSpecHorizontal);
		mBlurPipelineH->build();

		// Vertical pipeline
		mBlurPipelineV = lzvk::wrapper::ComputePipeline::create(mDevice);
		mBlurPipelineV->setShader(blurShader);
		mBlurPipelineV->setDescriptorSetLayouts({ mDescriptorSetLayout_Blur->getLayout() });
		mBlurPipelineV->setSpecializationConstant(0, sizeof(uint32_t), &kSpecVertical);
		mBlurPipelineV->build();
	}

	void Application::createCombinePipeline() {

		// 1 create pipeline
		mCombinePipeline = lzvk::wrapper::Pipeline::create(mDevice);

		mCombinePipeline->setColorAttachmentFormats({ mSwapChain->getFormat() });

		VkFormat depthFormat = mSwapChain->getDepthImageFormat();
		mCombinePipeline->setDepthAttachmentFormat(depthFormat);
		if (depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT) {
			mCombinePipeline->setStencilAttachmentFormat(depthFormat);
		}

		// 2.set shader group
		std::vector<lzvk::wrapper::Shader::Ptr> shaderGroup{};
		shaderGroup.push_back(lzvk::wrapper::Shader::create(mDevice, "shaders/ssao/quad_flip_vs.spv", VK_SHADER_STAGE_VERTEX_BIT, "main"));
		shaderGroup.push_back(lzvk::wrapper::Shader::create(mDevice, "shaders/ssao/combine_fs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main"));
		mCombinePipeline->setShaderGroup(shaderGroup);

		// 3.vertex
		mCombinePipeline->mVertexInputState.vertexBindingDescriptionCount = 0;
		mCombinePipeline->mVertexInputState.pVertexBindingDescriptions = nullptr;
		mCombinePipeline->mVertexInputState.vertexAttributeDescriptionCount = 0;
		mCombinePipeline->mVertexInputState.pVertexAttributeDescriptions = nullptr;

		// 4. Input assembly
		mCombinePipeline->mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mCombinePipeline->mAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		mCombinePipeline->mAssemblyState.primitiveRestartEnable = VK_FALSE;

		// 5. common state
		applyCommonPipelineState(mCombinePipeline, false, VK_CULL_MODE_NONE, PipelineType::Combine);

		// 8. Build Pipeline
		mCombinePipeline->build();
	}

	void Application::createCommandBuffers() {

		mCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {

			mCommandBuffers[i] = lzvk::wrapper::CommandBuffer::create(mDevice, mCommandPool);

		}
	}

	void Application::recordShadowPass(const lzvk::wrapper::CommandBuffer::Ptr& cmd) {

		cmd->transitionImageLayout(
			mImage_Shadow->getImage(),
			mImage_Shadow->getFormat(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
		);

		mFrameUniformManager->updateLight(mLight.getViewMatrix(), mLight.getProjectionMatrix(-74.45, 37.15, -54.96, 60.30, 41.56, -34.74), mDescriptorSet_Frame, mCurrentFrame);

		// --- Begin Render Pass ---
		cmd->beginRendering(mFramebuffer_Shadow);

		// --- Dynamic State ---
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(mShadowMapRes);
		viewport.height = static_cast<float>(mShadowMapRes);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		cmd->setViewport(0, viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { mShadowMapRes, mShadowMapRes };
		cmd->setScissor(0, scissor);

		// --- large scene ---
		cmd->bindGraphicPipeline(mShadowPipeline->getPipeline());
		cmd->setDepthBias(1.1f, 0.0f, 2.0f);
		cmd->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, mShadowPipeline->getLayout(), mDescriptorSet_Frame->getDescriptorSet(mCurrentFrame), 0);
		cmd->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, mShadowPipeline->getLayout(), mSceneMesh->getDescriptorSet_Static()->getDescriptorSet(0), 1);
		mSceneMesh->draw(cmd);

		cmd->disableDepthBias();
		cmd->endRendering();

	}

	void Application::transitionGeometryImages(const lzvk::wrapper::CommandBuffer::Ptr& cmd) {

			cmd->transitionImageLayout(
				mColorImage_Geometry->getImage(),
				mColorImage_Geometry->getFormat(),
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			);

			cmd->transitionImageLayout(
				mDepthImage_Geometry->getImage(),
				mDepthImage_Geometry->getFormat(),
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT

			);

	}

	void Application::recordGeometryPass(const lzvk::wrapper::CommandBuffer::Ptr& cmd) {

		transitionGeometryImages(cmd);

		cmd->transitionImageLayout(
			mImage_Shadow->getImage(),
			mImage_Shadow->getFormat(),
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
		);

		if (!mTexture_Shadow) {

			mTexture_Shadow = lzvk::renderer::Texture::create(mDevice, mImage_Shadow);
		}

		mShadowUniformManager->update(mDescriptorSet_Shadow, mTexture_Shadow, mCurrentFrame);

		// --- Begin Render Pass ---
		cmd->beginRendering(mFramebuffer_Geometry);

		// --- Dynamic State ---
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(mWidth);
		viewport.height = static_cast<float>(mHeight);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		cmd->setViewport(0, viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { mWidth, mHeight };
		cmd->setScissor(0, scissor);

		// --- skybox ---
		cmd->bindGraphicPipeline(mSkyboxPipeline->getPipeline());
		cmd->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, mSkyboxPipeline->getLayout(), mDescriptorSet_Frame->getDescriptorSet(mCurrentFrame), 0);
		cmd->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, mSkyboxPipeline->getLayout(), mDescriptorSet_Skybox->getDescriptorSet(0), 1);
		cmd->draw(36);

		// --- large scene ---
		cmd->bindGraphicPipeline(mSceneGraphPipeline->getPipeline());
		cmd->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, mSceneGraphPipeline->getLayout(), mDescriptorSet_Frame->getDescriptorSet(mCurrentFrame), 0);
		cmd->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, mSceneGraphPipeline->getLayout(), mSceneMesh->getDescriptorSet_Static()->getDescriptorSet(0), 1);
		cmd->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, mSceneGraphPipeline->getLayout(), mSceneMesh->getDescriptorSet_Diffuse()->getDescriptorSet(0), 2);
		cmd->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, mSceneGraphPipeline->getLayout(), mSceneMesh->getDescriptorSet_Emissive()->getDescriptorSet(0), 3);
		cmd->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, mSceneGraphPipeline->getLayout(), mSceneMesh->getDescriptorSet_Normal()->getDescriptorSet(0), 4);
		cmd->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, mSceneGraphPipeline->getLayout(), mSceneMesh->getDescriptorSet_Opacity()->getDescriptorSet(0), 5);
		cmd->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, mSceneGraphPipeline->getLayout(), mSceneMesh->getDescriptorSet_Specular()->getDescriptorSet(0), 6);
		cmd->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, mSceneGraphPipeline->getLayout(), mDescriptorSet_Shadow->getDescriptorSet(mCurrentFrame), 7);
		cmd->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, mSceneGraphPipeline->getLayout(), mDescriptorSet_Skybox->getDescriptorSet(0), 8);
		
		// push constants
		LightPushConstant pc{};
		pc.lightDir = glm::vec4(mLight.getDirection(), 0.0f);
		pc.cameraPos = glm::vec4(mCamera.getPosition(), 0.0f);
		cmd->pushConstants(mSceneGraphPipeline->getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, pc);
		
		mSceneMesh->draw(cmd);
		cmd->endRendering();

	}

	void Application::recordSSAOPass(const lzvk::wrapper::CommandBuffer::Ptr& cmd) {
		
		cmd->transitionImageLayout(
			mDepthImage_Geometry->getImage(),
			mDepthImage_Geometry->getFormat(),
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
		);

		if (!mDepthTexture_SSAO) {

			mDepthTexture_SSAO = lzvk::renderer::Texture::create(mDevice, mDepthImage_Geometry);
		}
	
		mSSAOUniformManager->update(mDescriptorSet_SSAO, mDepthTexture_SSAO, mRotationTexture, mAOImage_SSAO, mCurrentFrame);

		// 1. Layout transition for AO output image to GENERAL
		cmd->transitionImageLayout(
			mAOImage_SSAO->getImage(),
			mAOImage_SSAO->getFormat(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_GENERAL,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
		);


		// 2. Bind compute pipeline
		cmd->bindComputePipeline(mSSAOPipeline->getPipeline());
		cmd->bindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, mSSAOPipeline->getLayout(), mDescriptorSet_SSAO->getDescriptorSet(mCurrentFrame), 0);

		// 4. Push constants
		SSAOPushConstants pc{};
		pc.zNear = 0.1f;
		pc.zFar = 200.0f;
		pc.radius = 0.03f;
		pc.attScale = 0.95f;
		pc.distScale = 1.7f;
		cmd->pushConstants(mSSAOPipeline->getLayout(), VK_SHADER_STAGE_COMPUTE_BIT, pc);
	
		// 5. Dispatch compute shader
		uint32_t groupSizeX = (mWidth + 15) / 16;
		uint32_t groupSizeY = (mHeight + 15) / 16;
		cmd->dispatch(groupSizeX, groupSizeY, 1);
	}

	void Application::recordBlurPass(const lzvk::wrapper::CommandBuffer::Ptr& cmd) {


		auto pingImg = mAOImage_BlurPing;
		auto pongImg = mAOImage_BlurPong;


		if (!mDepthTexture_Blur)     mDepthTexture_Blur = lzvk::renderer::Texture::create(mDevice, mDepthImage_Geometry);
		if (!mAOTexture_SSAO)        mAOTexture_SSAO = lzvk::renderer::Texture::create(mDevice, mAOImage_SSAO);
		if (!mAOTexture_BlurPing)    mAOTexture_BlurPing = lzvk::renderer::Texture::create(mDevice, mAOImage_BlurPing);
		if (!mAOTexture_BlurPong)    mAOTexture_BlurPong = lzvk::renderer::Texture::create(mDevice, mAOImage_BlurPong);

		
		float mBlurDepthThreshold = 30.0f;

		lzvk::renderer::Texture::Ptr inputTex = mAOTexture_SSAO;

		// 1. Ping-pong blur passes
		for (int i = 0; i < mBlurPassCount; ++i) {

			cmd->transitionImageLayout(
				inputTex->getImage()->getImage(),
				inputTex->getImage()->getFormat(),
				VK_IMAGE_LAYOUT_GENERAL,  // or keep track of last layout
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);


			// Horizontal pass: inputTex → ping
			cmd->transitionImageLayout(pingImg->getImage(), pingImg->getFormat(),
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			mBlurUniformManager->update(mDescriptorSet_BlurH[i], mDepthTexture_Blur, inputTex, pingImg, mCurrentFrame);

			cmd->bindComputePipeline(mBlurPipelineH->getPipeline());
			cmd->bindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, mBlurPipelineH->getLayout(), mDescriptorSet_BlurH[i]->getDescriptorSet(mCurrentFrame), 0);

			BlurPushConstant pc{};
			pc.depthThreshold = mBlurDepthThreshold;
			cmd->pushConstants(mBlurPipelineH->getLayout(), VK_SHADER_STAGE_COMPUTE_BIT, pc);

			cmd->dispatch((mWidth + 15) / 16, (mHeight + 15) / 16, 1);

			inputTex = mAOTexture_BlurPing;

			cmd->transitionImageLayout(
				inputTex->getImage()->getImage(), inputTex->getImage()->getFormat(),
				VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			// Vertical pass: inputTex → pong
			cmd->transitionImageLayout(pongImg->getImage(), pongImg->getFormat(),
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			mBlurUniformManager->update(mDescriptorSet_BlurV[i], mDepthTexture_Blur, inputTex, pongImg, mCurrentFrame);

			cmd->bindComputePipeline(mBlurPipelineV->getPipeline());
			cmd->bindDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, mBlurPipelineV->getLayout(), mDescriptorSet_BlurV[i]->getDescriptorSet(mCurrentFrame), 0);

			cmd->pushConstants(mBlurPipelineV->getLayout(), VK_SHADER_STAGE_COMPUTE_BIT, pc);

			cmd->dispatch((mWidth + 15) / 16, (mHeight + 15) / 16, 1);

			inputTex = mAOTexture_BlurPong;
		}

		mFinalAOImage = inputTex->getImage();
	}

	void Application::recordCombinePass(const lzvk::wrapper::CommandBuffer::Ptr& cmd, uint32_t imageIndex) {
		
		// 1. Transition render target
		cmd->transitionImageLayout(
			mSwapChain->getImage(imageIndex),
			mSwapChain->getFormat(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		);

		cmd->transitionImageLayout(
			mColorImage_Geometry->getImage(),
			mColorImage_Geometry->getFormat(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
		);

		cmd->transitionImageLayout(
			mFinalAOImage->getImage(),
			mFinalAOImage->getFormat(),
			VK_IMAGE_LAYOUT_GENERAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
		);


		if (!mColorTexture_Combine) {
			mColorTexture_Combine = renderer::Texture::create(mDevice, mColorImage_Geometry);
		}

		if (!mAOTexture_Combine) {
			mAOTexture_Combine = renderer::Texture::create(mDevice, mFinalAOImage);
		}

		mCombineUniformManager->update(mDescriptorSet_Combine, mColorTexture_Combine, mAOTexture_Combine, mCurrentFrame);


		// 2. Begin render
		cmd->beginRendering(mSwapChain, imageIndex);

		// --- Dynamic State ---
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(mWidth);
		viewport.height = static_cast<float>(mHeight);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		cmd->setViewport(0, viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { mWidth, mHeight };
		cmd->setScissor(0, scissor);

		// 3. Bind pipeline + descriptor sets
		cmd->bindGraphicPipeline(mCombinePipeline->getPipeline());
		cmd->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, mCombinePipeline->getLayout(), mDescriptorSet_Combine->getDescriptorSet(mCurrentFrame), 0);

		// 4. Push constant
		CombinePushConstant pc{};
		pc.scale = 1.5f;
		pc.bias = 0.16f;
		cmd->pushConstants(mCombinePipeline->getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, pc);

		// 5. Draw fullscreen triangle
		cmd->draw(3);

		// 6. End render
		cmd->endRendering();


	}

	void Application::recordImGuiPass(const lzvk::wrapper::CommandBuffer::Ptr& cmd, uint32_t imageIndex) {

		// 2. Begin a new render pass for ImGui
		cmd->beginRenderingForImGui(
			mSwapChain->getImageView(imageIndex),
			mSwapChain->getFormat(),
			mSwapChain->getExtent()
		);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(mSwapChain->getExtent().width);
		viewport.height = static_cast<float>(mSwapChain->getExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		cmd->setViewport(0, viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = mSwapChain->getExtent();
		cmd->setScissor(0, scissor);

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Light Control");
		//ImGui::SliderFloat("Theta", &mLightTheta, 0.0f, 360.0f);
		//ImGui::SliderFloat("Phi", &mLightPhi, -89.9f, 89.9f);
		ImGui::End();

		mLightTheta += 0.05f;
		if (mLightTheta > 360.0f)
			mLightTheta -= 360.0f;

		mLight.setDirectionFromSpherical(mLightTheta, mLightPhi);

		ImGui::Render();

		// 3. Issue ImGui draw calls
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd->getCommandBuffer());

		// 4. End render pass
		cmd->endRendering();

	}

	void Application::recordCommandBuffer(uint32_t imageIndex) {

		mCommandBuffers[mCurrentFrame]->begin();

		recordShadowPass(mCommandBuffers[mCurrentFrame]);

		recordGeometryPass(mCommandBuffers[mCurrentFrame]);

		recordSSAOPass(mCommandBuffers[mCurrentFrame]);

		recordBlurPass(mCommandBuffers[mCurrentFrame]);

		recordCombinePass(mCommandBuffers[mCurrentFrame], imageIndex);

		recordImGuiPass(mCommandBuffers[mCurrentFrame], imageIndex);

		mCommandBuffers[mCurrentFrame]->transitionImageLayout(
			mSwapChain->getImage(imageIndex),
			mSwapChain->getFormat(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
		);


		mCommandBuffers[mCurrentFrame]->end();
	}

	void Application::createSyncObjects() {

		mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mRenderFinishedSemaphores.resize(mSwapChain->getImageCount());
		mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			mImageAvailableSemaphores[i] = lzvk::wrapper::Semaphore::create(mDevice);
			mInFlightFences[i] = lzvk::wrapper::Fence::create(mDevice);
		}

		for (int i = 0; i < mSwapChain->getImageCount(); ++i) {

			mRenderFinishedSemaphores[i] = lzvk::wrapper::Semaphore::create(mDevice);
		}

	}

	void Application::recreateSwapChain() {
		
		// 1 Check if the window is minimized
		int width = 0, height = 0;

		glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);
		while (width == 0 || height == 0) {
			
			glfwWaitEvents();
			glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);
		}

		// 2 Wait task
		vkDeviceWaitIdle(mDevice->getDevice());

		// 3 Clean up
		cleanupSwapChain();

		// Swap chain
		mSwapChain = lzvk::wrapper::SwapChain::create(mDevice, mWindow, mSurface, mCommandPool);
		mWidth = mSwapChain->getExtent().width;
		mHeight = mSwapChain->getExtent().height;

	}

	void Application::cleanupSwapChain() {
		
		mSwapChain.reset();
	
	}

	void Application::mainLoop() {

		while (!mWindow->shouldClose()) {

			mWindow->pollEvents();
			mWindow->processEvent();

			render();
		}

		vkDeviceWaitIdle(mDevice->getDevice());
	}

	void Application::render() {

		mInFlightFences[mCurrentFrame]->block();

		// 1 Get next frame
		uint32_t imageIndex{ 0 };
		VkResult result = vkAcquireNextImageKHR(mDevice->getDevice(), mSwapChain->getSwapChain(), UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame]->getSemaphore(), VK_NULL_HANDLE, &imageIndex);

		// 1.1 Check if need recreate swap chain
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			
			recreateSwapChain();
			mWindow->mWindowResized = false;

		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			
			throw std::runtime_error("Error: failed to acquire next image");
		}

		// 1.2 Update 
		mFrameUniformManager->update(mCamera.getViewMatrix(), mCamera.getProjectMatrix(), mDescriptorSet_Frame, mCurrentFrame);
		mInFlightFences[mCurrentFrame]->resetFence();

		// 1.3 Record commands
		vkResetCommandBuffer(mCommandBuffers[mCurrentFrame]->getCommandBuffer(), 0);
		recordCommandBuffer(imageIndex);

		// 2 Submit info
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		// 2.1 Wait semaphore
		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame]->getSemaphore() };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		// 2.2 command buffer
		auto commandBuffer = mCommandBuffers[mCurrentFrame]->getCommandBuffer();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		// 2.3 Signal semaphore
		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[imageIndex]->getSemaphore() };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		// 2.4 Submit render command
		if (vkQueueSubmit(mDevice->getGraphicQueue(), 1, &submitInfo, mInFlightFences[mCurrentFrame]->getFence()) != VK_SUCCESS) {

			throw std::runtime_error("Error: failed to submit renderCommand");
		}

		// 3 Present info
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		// 3.1 Swap chain
		VkSwapchainKHR swapChains[] = { mSwapChain->getSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		// 3.2 Submit present command
		result = vkQueuePresentKHR(mDevice->getPresentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mWindow->mWindowResized) {
			recreateSwapChain();
			mWindow->mWindowResized = false;
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to present");
		}

		mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void Application::cleanUp() {

		// Swap chain
		mSwapChain.reset();

		// === Combine ===
		mCombinePipeline.reset();
		mAOTexture_Combine.reset();
		mColorTexture_Combine.reset();
		mDescriptorSet_Combine.reset();
		mDescriptorPool_Combine.reset();
		mDescriptorSetLayout_Combine.reset();
		mCombineUniformManager.reset();

		// === Blur ===
		mBlurPipelineH.reset();
		mBlurPipelineV.reset();
		mAOImage_BlurPing.reset();
		mAOImage_BlurPong.reset();
		mFinalAOImage.reset();
		mAOTexture_SSAO.reset();
		mAOTexture_BlurPing.reset();
		mAOTexture_BlurPong.reset();
		mDepthTexture_Blur.reset();

		for (auto& set : mDescriptorSet_BlurH) set.reset();
		for (auto& set : mDescriptorSet_BlurV) set.reset();
		mDescriptorPool_Blur.reset();
		mDescriptorSetLayout_Blur.reset();
		mBlurUniformManager.reset();

		// === SSAO ===
		mSSAOPipeline.reset();
		mAOImage_SSAO.reset();
		mRotationTexture.reset();
		mDepthTexture_SSAO.reset();
		mDescriptorSet_SSAO.reset();
		mDescriptorPool_SSAO.reset();
		mDescriptorSetLayout_SSAO.reset();
		mSSAOUniformManager.reset();

		// === Geometry ===
		mFramebuffer_Geometry.reset();
		mColorImage_Geometry.reset();
		mDepthImage_Geometry.reset();

		// === Scene ===
		mSceneMesh.reset();
		mSceneGraphPipeline.reset();
		mShadowUniformManager.reset();

		// === Skybox ===
		mSkyboxPipeline.reset();
		mDescriptorSet_Skybox.reset();
		mDescriptorPool_Skybox.reset();
		mDescriptorSetLayout_Skybox.reset();
		mSkyboxUniformManager.reset();

		// === Shadow ===
		mShadowPipeline.reset();
		mTexture_Shadow.reset();
		mDescriptorSet_Shadow.reset();
		mDescriptorPool_Shadow.reset();
		mDescriptorSetLayout_Shadow.reset();
		mImage_Shadow.reset();
		mFramebuffer_Shadow.reset();

		// === Frame ===
		mDescriptorSet_Frame.reset();
		mDescriptorPool_Frame.reset();
		mDescriptorSetLayout_Frame.reset();
		mFrameUniformManager.reset();

		// === Command / Sync ===
		mCommandBuffers.clear();
		mImageAvailableSemaphores.clear();
		mRenderFinishedSemaphores.clear();
		mInFlightFences.clear();

		cleanUpImGui();
		// === Core Vulkan handles ===
		mCommandPool.reset();
		mDevice.reset();
		mSurface.reset();
		mInstance.reset();
		mWindow.reset();

	}
}