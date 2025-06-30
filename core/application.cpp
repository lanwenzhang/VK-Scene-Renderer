#include "application.h"


namespace LZ::Core {

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
		mCamera.setSpeed(0.001f);

	}

	void Application::initVulkan() {

		// 1 Set up environment
		mInstance = LZ::Wrapper::Instance::create(true);
		mSurface = Wrapper::WindowSurface::create(mInstance, mWindow);
		mDevice = Wrapper::Device::create(mInstance, mSurface);
		mCommandPool = Wrapper::CommandPool::create(mDevice);

		mSwapChain = Wrapper::SwapChain::create(mDevice, mWindow, mSurface, mCommandPool);
		mWidth = mSwapChain->getExtent().width;
		mHeight = mSwapChain->getExtent().height;

		// 2 Set up pipeline
		mRenderPass = Wrapper::RenderPass::create(mDevice);
		createRenderPass();
		mSwapChain->createFrameBuffers(mRenderPass);
		
		loadMeshFile("assets/crytek_sponza/sponza.obj", mMeshData, mScene);
		mSceneMesh = LZ::Renderer::SceneMeshRenderer::create(mDevice, mCommandPool, mMeshData, mScene);

		createDescriptorSets();

		mSkyboxPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
		createSkyboxPipeline();

		mSceneGraphPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
		createSceneGraphPipeline();

		createCommandBuffers();
		createSyncObjects();

	}

	void Application::createDescriptorSets()
	{
		// ----------- Frame Uniform -----------
		mFrameUniformManager = LZ::Renderer::FrameUniformManager::create();
		mFrameUniformManager->init(mDevice, MAX_FRAMES_IN_FLIGHT);

		std::vector<LZ::Wrapper::UniformParameter::Ptr> frameParams =
			mFrameUniformManager->getParams();

		mDescriptorSetLayout_Frame = LZ::Wrapper::DescriptorSetLayout::create(mDevice);
		mDescriptorSetLayout_Frame->build(frameParams);

		mDescriptorPool_Frame = LZ::Wrapper::DescriptorPool::create(mDevice);
		mDescriptorPool_Frame->build(frameParams, MAX_FRAMES_IN_FLIGHT);

		mDescriptorSet_Frame = LZ::Wrapper::DescriptorSet::create(mDevice, frameParams, mDescriptorSetLayout_Frame, mDescriptorPool_Frame, MAX_FRAMES_IN_FLIGHT);

		// ----------- Static Uniforms -----------
		mTransformUniformManager = LZ::Renderer::TransformUniformManager::create();
		mTransformUniformManager->init(mDevice, mScene.globalTransform.size(), mScene.globalTransform.empty() ? nullptr : mScene.globalTransform.data(), MAX_FRAMES_IN_FLIGHT);

		mMaterialUniformManager = LZ::Renderer::MaterialUniformManager::create();
		mMaterialUniformManager->init(mDevice, mMeshData.materials.size(), mMeshData.materials.empty() ? nullptr : mMeshData.materials.data(), MAX_FRAMES_IN_FLIGHT);

		mSkyboxUniformManager = LZ::Renderer::SkyboxUniformManager::create();
		mSkyboxUniformManager->init(mDevice, mCommandPool, "assets/piazza_bologni_1k.hdr");

		mDrawDataUniformManager = LZ::Renderer::DrawDataUniformManager::create();
		mDrawDataUniformManager->init(mDevice, mScene.drawDataArray.size(), mScene.drawDataArray.data(), MAX_FRAMES_IN_FLIGHT);


		std::vector<LZ::Wrapper::UniformParameter::Ptr> staticParams;

		auto append = [&](const std::vector<LZ::Wrapper::UniformParameter::Ptr>& params) {
			staticParams.insert(staticParams.end(), params.begin(), params.end());
			};

		append(mTransformUniformManager->getParams());
		append(mMaterialUniformManager->getParams());
		append(mSkyboxUniformManager->getParams());
		append(mDrawDataUniformManager->getParams());


		for (auto& p : staticParams) {
			std::cout << "[DEBUG] Binding " << p->mBinding
				<< ", Count = " << p->mCount
				<< ", DescriptorType = " << p->mDescriptorType << std::endl;
		}

		mDescriptorSetLayout_Static = Wrapper::DescriptorSetLayout::create(mDevice);
		mDescriptorSetLayout_Static->build(staticParams);

		mDescriptorPool_Static = Wrapper::DescriptorPool::create(mDevice);
		mDescriptorPool_Static->build(staticParams, 1);

		mDescriptorSet_Static = Wrapper::DescriptorSet::create(
			mDevice, staticParams,
			mDescriptorSetLayout_Static,
			mDescriptorPool_Static,
			1
		);

		// ------ TEXTURE DESCRIPTOR SETS ------
		mSceneTextureManager = LZ::Renderer::SceneTextureManager::create();
		mSceneTextureManager->init(mDevice, mCommandPool, mSceneMesh, MAX_FRAMES_IN_FLIGHT);

		auto diffuseParams = mSceneTextureManager->getDiffuseParams();
		mDescriptorSetLayout_Diffuse = Wrapper::DescriptorSetLayout::create(mDevice);
		mDescriptorSetLayout_Diffuse->build(diffuseParams);

		mDescriptorPool_Diffuse = Wrapper::DescriptorPool::create(mDevice);
		mDescriptorPool_Diffuse->build(diffuseParams, 1);

		mDescriptorSet_Diffuse = Wrapper::DescriptorSet::create(
			mDevice, diffuseParams,
			mDescriptorSetLayout_Diffuse,
			mDescriptorPool_Diffuse,
			1);


		/*auto emissiveParams = mSceneTextureManager->getEmissiveParams();
		mDescriptorSetLayout_Emissive = Wrapper::DescriptorSetLayout::create(mDevice);
		mDescriptorSetLayout_Emissive->build(emissiveParams);

		mDescriptorPool_Emissive = Wrapper::DescriptorPool::create(mDevice);
		mDescriptorPool_Emissive->build(emissiveParams, 1);

		mDescriptorSet_Emissive = Wrapper::DescriptorSet::create(
			mDevice, emissiveParams,
			mDescriptorSetLayout_Emissive,
			mDescriptorPool_Emissive,
			1
		);

		auto occlusionParams = mSceneTextureManager->getOcclusionParams();
		mDescriptorSetLayout_Occlusion = Wrapper::DescriptorSetLayout::create(mDevice);
		mDescriptorSetLayout_Occlusion->build(occlusionParams);

		mDescriptorPool_Occlusion = Wrapper::DescriptorPool::create(mDevice);
		mDescriptorPool_Occlusion->build(occlusionParams, 1);

		mDescriptorSet_Occlusion = Wrapper::DescriptorSet::create(
			mDevice, occlusionParams,
			mDescriptorSetLayout_Occlusion,
			mDescriptorPool_Occlusion,
			1
		);*/


		mDescriptorSetLayout_Static = Wrapper::DescriptorSetLayout::create(mDevice);

		mDescriptorSetLayout_Static->build(staticParams);

		mDescriptorPool_Static = Wrapper::DescriptorPool::create(mDevice);

		mDescriptorPool_Static->build(staticParams, 1);

		mDescriptorSet_Static = Wrapper::DescriptorSet::create(mDevice, staticParams, mDescriptorSetLayout_Static, mDescriptorPool_Static, 1);
	}

	void Application::applyCommonPipelineState(const Wrapper::Pipeline::Ptr& pipeline, bool enableDepthWrite, VkCullModeFlagBits cullMode, bool isSceneGraph) {
		
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
				mDescriptorSetLayout_Static->getLayout(),
				mDescriptorSetLayout_Diffuse->getLayout(),
				//mDescriptorSetLayout_Emissive->getLayout(),
				//mDescriptorSetLayout_Occlusion->getLayout()
			};
		}
		else {
			pipeline->mSetLayoutsStorage = {
				mDescriptorSetLayout_Frame->getLayout(),
				mDescriptorSetLayout_Static->getLayout()
			};
		}

		std::cout << "Pipeline using set count = " << pipeline->mSetLayoutsStorage.size() << std::endl;
		pipeline->mLayoutState.setLayoutCount = static_cast<uint32_t>(pipeline->mSetLayoutsStorage.size());
		pipeline->mLayoutState.pSetLayouts = pipeline->mSetLayoutsStorage.data();
		pipeline->mLayoutState.flags = 0;;
	}

	void Application::createSkyboxPipeline() {

		// Shaders
		std::vector<Wrapper::Shader::Ptr> shaderGroup{};
		shaderGroup.push_back(Wrapper::Shader::create(mDevice, "shaders/cubemapvs.spv", VK_SHADER_STAGE_VERTEX_BIT, "main"));
		shaderGroup.push_back(Wrapper::Shader::create(mDevice, "shaders/cubemapfs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main"));
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

		std::vector<Wrapper::Shader::Ptr> shaderGroup{};
		shaderGroup.push_back(Wrapper::Shader::create(mDevice, "shaders/sceneGraphvs.spv", VK_SHADER_STAGE_VERTEX_BIT, "main"));
		shaderGroup.push_back(Wrapper::Shader::create(mDevice, "shaders/sceneGraphfs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main"));
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
		depthAttachmentDes.format = Wrapper::Image::findDepthFormat(mDevice);
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
		Wrapper::SubPass subPass{};
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

			mCommandBuffers[i] = Wrapper::CommandBuffer::create(mDevice, mCommandPool);

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

		// --- Skybox ---
		mCommandBuffers[mCurrentFrame]->bindGraphicPipeline(mSkyboxPipeline->getPipeline());
		mCommandBuffers[mCurrentFrame]->bindDescriptorSet(mSkyboxPipeline->getLayout(), mDescriptorSet_Frame->getDescriptorSet(mCurrentFrame), 0);
		mCommandBuffers[mCurrentFrame]->bindDescriptorSet(mSkyboxPipeline->getLayout(), mDescriptorSet_Static->getDescriptorSet(0), 1);
		mCommandBuffers[mCurrentFrame]->draw(36);

		// --- Indirect Model ---
		mCommandBuffers[mCurrentFrame]->bindGraphicPipeline(mSceneGraphPipeline->getPipeline());
		mCommandBuffers[mCurrentFrame]->bindDescriptorSet(mSceneGraphPipeline->getLayout(), mDescriptorSet_Frame->getDescriptorSet(mCurrentFrame),0);
		mCommandBuffers[mCurrentFrame]->bindDescriptorSet(mSceneGraphPipeline->getLayout(), mDescriptorSet_Static->getDescriptorSet(0), 1);
		mCommandBuffers[mCurrentFrame]->bindDescriptorSet(mSceneGraphPipeline->getLayout(), mDescriptorSet_Diffuse->getDescriptorSet(0), 2);
		//mCommandBuffers[mCurrentFrame]->bindDescriptorSet(mSceneGraphPipeline->getLayout(), mDescriptorSet_Emissive->getDescriptorSet(0), 3);
		//mCommandBuffers[mCurrentFrame]->bindDescriptorSet(mSceneGraphPipeline->getLayout(), mDescriptorSet_Occlusion->getDescriptorSet(0), 4);
		mSceneMesh->draw(mCommandBuffers[mCurrentFrame]);
		mCommandBuffers[mCurrentFrame]->endRenderPass();
		mCommandBuffers[mCurrentFrame]->end();

	}

	void Application::createSyncObjects() {

		mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			mImageAvailableSemaphores[i] = Wrapper::Semaphore::create(mDevice);
			mRenderFinishedSemaphores[i] = Wrapper::Semaphore::create(mDevice);
			mInFlightFences[i] = Wrapper::Fence::create(mDevice);
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
		mSwapChain = Wrapper::SwapChain::create(mDevice, mWindow, mSurface, mCommandPool);
		mWidth = mSwapChain->getExtent().width;
		mHeight = mSwapChain->getExtent().height;

		//// Render pass
		//mRenderPass = Wrapper::RenderPass::create(mDevice);
		//createRenderPass();

		mSwapChain->createFrameBuffers(mRenderPass);

		//// 3 Start render
		//// 3.1 Pipeline
		//mSkyboxPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
		//createSkyboxPipeline();
		//
		//mSceneGraphPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
		//createSceneGraphPipeline();

		//// 3.2 Command buffer
		//createCommandBuffers();

		//// 3.3 Sync
		//createSyncObjects();

	}

	void Application::cleanupSwapChain() {
		
		mSwapChain.reset();
		//mCommandBuffers.clear();
		//mSkyboxPipeline.reset();
		//mSceneGraphPipeline.reset();
		//mRenderPass.reset();

		//mImageAvailableSemaphores.clear();
		//mRenderFinishedSemaphores.clear();
		//mInFlightFences.clear();
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
		mCamera.updatePitch(0.001f);
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
		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame]->getSemaphore() };
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

		mDescriptorSet_Static.reset();
		mDescriptorPool_Static.reset();
		mDescriptorSetLayout_Static.reset();

		// Uniform Managers
		mFrameUniformManager.reset();
		mTransformUniformManager.reset();
		mMaterialUniformManager.reset();
		mSkyboxUniformManager.reset();
		mSceneTextureManager.reset();

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