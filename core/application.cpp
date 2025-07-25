#include "application.h"
#include "../tools/scene_tools.h"


namespace lzvk::core {

	void Application::run() {

		initWindow();
		initVulkan();
		mainLoop();
		cleanUp();
	}

	void Application::onMouseMove(double xpos, double ypos) {
		
		mCamera.onMouseMove(xpos, ypos);
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
		mCamera.setPerpective(45.0f, (float)mWidth / (float)mHeight, 0.1f, 2000.0f);
		mCamera.setSpeed(0.01f);

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

		// 2 Set up pipeline
		mRenderPass = lzvk::wrapper::RenderPass::create(mDevice);
		createRenderPass();
		mSwapChain->createFrameBuffers(mRenderPass);
		
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

		//mSceneInterior.localTransform[0] = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f));
		//lzvk::loader::recalculateGlobalTransforms(mSceneInterior);

		/*mSceneMesh = lzvk::renderer::SceneMeshRenderer::create(mDevice, mCommandPool, mMeshDataExterior, mSceneExterior, MAX_FRAMES_IN_FLIGHT);*/
		//mSceneMesh = lzvk::renderer::SceneMeshRenderer::create(mDevice, mCommandPool, mMeshDataInterior, mSceneInterior, MAX_FRAMES_IN_FLIGHT);
		mSceneMesh = lzvk::renderer::SceneMeshRenderer::create(mDevice, mCommandPool, mMeshData, mScene, MAX_FRAMES_IN_FLIGHT);

		createDescriptorSets();

		mSkyboxPipeline = lzvk::wrapper::Pipeline::create(mDevice, mRenderPass);
		createSkyboxPipeline();

		mSceneGraphPipeline = lzvk::wrapper::Pipeline::create(mDevice, mRenderPass);
		createSceneGraphPipeline();

		createCommandBuffers();
		createSyncObjects();

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
		// ========== Skybox Uniform ==========
		//

		mSkyboxUniformManager = lzvk::renderer::SkyboxUniformManager::create();
		mSkyboxUniformManager->init(mDevice, mCommandPool, "assets/piazza_bologni_1k.hdr");

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
	}

	void Application::applyCommonPipelineState(const lzvk::wrapper::Pipeline::Ptr& pipeline, bool enableDepthWrite, VkCullModeFlagBits cullMode, bool isSceneGraph) {
		
		// Dynamic state
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		pipeline->setDynamicStates(dynamicStates);

		// Rasterization
		pipeline->mRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipeline->mRasterState.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline->mRasterState.lineWidth = 1.0f;
		pipeline->mRasterState.cullMode = cullMode;
		pipeline->mRasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		pipeline->mRasterState.depthBiasEnable = VK_FALSE;
		pipeline->mRasterState.depthBiasConstantFactor = 0.0f;
		pipeline->mRasterState.depthBiasClamp = 0.0f;
		pipeline->mRasterState.depthBiasSlopeFactor = 0.0f;

		// MSAA
		pipeline->mSampleState.sampleShadingEnable = VK_FALSE;
		pipeline->mSampleState.rasterizationSamples = mDevice->getMaxUsableSampleCount();
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

		if (isSceneGraph) {
			pipeline->mSetLayoutsStorage = {

				 mDescriptorSetLayout_Frame->getLayout(),
				 mSceneMesh->getDescriptorSetLayout_Static()->getLayout(),
				 mSceneMesh->getDescriptorSetLayout_Diffuse()->getLayout(),
				 mSceneMesh->getDescriptorSetLayout_Emissive()->getLayout(),
				 mSceneMesh->getDescriptorSetLayout_Normal()->getLayout()
			};
		}
		else {
			pipeline->mSetLayoutsStorage = {
				mDescriptorSetLayout_Frame->getLayout(),
				mDescriptorSetLayout_Skybox->getLayout()
			};
		}

		std::cout << "Pipeline using set count = " << pipeline->mSetLayoutsStorage.size() << std::endl;
		pipeline->mLayoutState.setLayoutCount = static_cast<uint32_t>(pipeline->mSetLayoutsStorage.size());
		pipeline->mLayoutState.pSetLayouts = pipeline->mSetLayoutsStorage.data();
		pipeline->mLayoutState.flags = 0;;
	}

	void Application::createSkyboxPipeline() {

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
		applyCommonPipelineState(mSkyboxPipeline, false, VK_CULL_MODE_FRONT_BIT, false);

		// Render pass
		mSkyboxPipeline->build();
	}

	void Application::createSceneGraphPipeline() {

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

		applyCommonPipelineState(mSceneGraphPipeline, true, VK_CULL_MODE_BACK_BIT, true);

		mSceneGraphPipeline->build();
	}

	void Application::createRenderPass() {

		// 1.1 Resolve color attachment
		VkAttachmentDescription finalAttachmentDes{};
		finalAttachmentDes.format = mSwapChain->getFormat();
		finalAttachmentDes.samples = VK_SAMPLE_COUNT_1_BIT;
		finalAttachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		finalAttachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		finalAttachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		finalAttachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		finalAttachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		finalAttachmentDes.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		mRenderPass->addAttachment(finalAttachmentDes);

		// 1.2 Multi-sample color attachment
		VkAttachmentDescription MutiAttachmentDes{};
		MutiAttachmentDes.format = mSwapChain->getFormat();
		MutiAttachmentDes.samples = mDevice->getMaxUsableSampleCount();
		MutiAttachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		MutiAttachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		MutiAttachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		MutiAttachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		MutiAttachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		MutiAttachmentDes.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		mRenderPass->addAttachment(MutiAttachmentDes);

		// 1.3 Depth attachment
		VkAttachmentDescription depthAttachmentDes{};
		depthAttachmentDes.format = lzvk::wrapper::Image::findDepthFormat(mDevice);
		depthAttachmentDes.samples = mDevice->getMaxUsableSampleCount();
		depthAttachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachmentDes.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		mRenderPass->addAttachment(depthAttachmentDes);

		// 2 Attachment reference
		VkAttachmentReference finalAttachmentRef{};
		finalAttachmentRef.attachment = 0;
		finalAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference mutiAttachmentRef{};
		mutiAttachmentRef.attachment = 1;
		mutiAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 2;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// 3 Subpass 
		lzvk::wrapper::SubPass subPass{};
		subPass.addColorAttachmentReference(mutiAttachmentRef);
		subPass.setDepthStencilAttachmentReference(depthAttachmentRef);
		subPass.setResolveAttachmentReference(finalAttachmentRef);

		subPass.buildSubPassDescription();

		mRenderPass->addSubPass(subPass);

		// 4 Subpass dependency
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		mRenderPass->addDependency(dependency);

		// 5 build renderpass
		mRenderPass->buildRenderPass();


	}

	void Application::createCommandBuffers() {

		mCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {

			mCommandBuffers[i] = lzvk::wrapper::CommandBuffer::create(mDevice, mCommandPool);

		}
	}

	void Application::recordCommandBuffer(uint32_t imageIndex) {

		mCommandBuffers[mCurrentFrame]->begin();

		// --- Begin Render Pass ---
		VkRenderPassBeginInfo renderBeginInfo{};
		renderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderBeginInfo.renderPass = mRenderPass->getRenderPass();
		renderBeginInfo.framebuffer = mSwapChain->getFrameBuffer(imageIndex);
		renderBeginInfo.renderArea.offset = { 0, 0 };
		renderBeginInfo.renderArea.extent = mSwapChain->getExtent();

		std::vector<VkClearValue> clearValues{};
		clearValues.push_back({ {0.09f, 0.125f, 0.192f, 1.0f} });
		clearValues.push_back({ {0.09f, 0.125f, 0.192f, 1.0f} });
		clearValues.push_back({ {1.0f, 0} });
		renderBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderBeginInfo.pClearValues = clearValues.data();

		mCommandBuffers[mCurrentFrame]->beginRenderPass(renderBeginInfo);

		// --- Dynamic State ---
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(mWidth);
		viewport.height = static_cast<float>(mHeight);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		mCommandBuffers[mCurrentFrame]->setViewport(0, viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { mWidth, mHeight };
		mCommandBuffers[mCurrentFrame]->setScissor(0, scissor);

		// --- skybox ---
		mCommandBuffers[mCurrentFrame]->bindGraphicPipeline(mSkyboxPipeline->getPipeline());
		mCommandBuffers[mCurrentFrame]->bindDescriptorSet(mSkyboxPipeline->getLayout(), mDescriptorSet_Frame->getDescriptorSet(mCurrentFrame), 0);
		mCommandBuffers[mCurrentFrame]->bindDescriptorSet(mSkyboxPipeline->getLayout(), mDescriptorSet_Skybox->getDescriptorSet(0), 1);
		mCommandBuffers[mCurrentFrame]->draw(36);

		// --- large scene ---
		mCommandBuffers[mCurrentFrame]->bindGraphicPipeline(mSceneGraphPipeline->getPipeline());
		mCommandBuffers[mCurrentFrame]->bindDescriptorSet(mSceneGraphPipeline->getLayout(), mDescriptorSet_Frame->getDescriptorSet(mCurrentFrame),0);
		mCommandBuffers[mCurrentFrame]->bindDescriptorSet(mSceneGraphPipeline->getLayout(), mSceneMesh->getDescriptorSet_Static()->getDescriptorSet(0), 1);
		mCommandBuffers[mCurrentFrame]->bindDescriptorSet(mSceneGraphPipeline->getLayout(), mSceneMesh->getDescriptorSet_Diffuse()->getDescriptorSet(0), 2);
		mCommandBuffers[mCurrentFrame]->bindDescriptorSet(mSceneGraphPipeline->getLayout(), mSceneMesh->getDescriptorSet_Emissive()->getDescriptorSet(0), 3);
		mCommandBuffers[mCurrentFrame]->bindDescriptorSet(mSceneGraphPipeline->getLayout(), mSceneMesh->getDescriptorSet_Normal()->getDescriptorSet(0), 4);
		mSceneMesh->draw(mCommandBuffers[mCurrentFrame]);
		mCommandBuffers[mCurrentFrame]->endRenderPass();
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

		mSwapChain->createFrameBuffers(mRenderPass);

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
		//mCamera.updatePitch(0.001f);
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

		// Swapchain first
		mSwapChain.reset();

		// Descriptor Sets
		mDescriptorSet_Frame.reset();
		mDescriptorPool_Frame.reset();
		mDescriptorSetLayout_Frame.reset();

		mDescriptorSet_Skybox.reset();
		mDescriptorPool_Skybox.reset();
		mDescriptorSetLayout_Skybox.reset();

		// Uniform Managers
		mFrameUniformManager.reset();
		mSkyboxUniformManager.reset();
	
		mSkyboxPipeline.reset();
		mSceneGraphPipeline.reset();
		mRenderPass.reset();

		mImageAvailableSemaphores.clear();
		mRenderFinishedSemaphores.clear();
		mInFlightFences.clear();

		mSceneMesh.reset();
	
		mCommandPool.reset();
		mDevice.reset();
		mSurface.reset();
		mInstance.reset();
		mWindow.reset();

	}
}