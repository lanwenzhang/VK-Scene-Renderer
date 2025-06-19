#include "application.h"
#include "vulkanWrapper/image.h"

namespace FF {

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

	void Application::initWindow() {

		mWindow = Wrapper::Window::create(mWidth, mHeight);
		mWindow->setApp(shared_from_this());

		mCamera.lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		mCamera.update();
		mCamera.setPerpective(45.0f, (float)mWidth / (float)mHeight, 0.1f, 100.0f);
		mCamera.setSpeed(0.001f);

	}

	void Application::initVulkan() {

		// 1 Set up environment
		// 1.1 Instance
		mInstance = Wrapper::Instance::create(true);

		// 1.2 Surface
		mSurface = Wrapper::WindowSurface::create(mInstance, mWindow);

		// 1.3 Device
		mDevice = Wrapper::Device::create(mInstance, mSurface);

		// 3.1 Command pool
		mCommandPool = Wrapper::CommandPool::create(mDevice);

		// 2 Prepare render target
		// 2.1 Swap chain
		mSwapChain = Wrapper::SwapChain::create(mDevice, mWindow, mSurface, mCommandPool);
		mWidth = mSwapChain->getExtent().width;
		mHeight = mSwapChain->getExtent().height;

		// 2.2 Render pass
		mRenderPass = Wrapper::RenderPass::create(mDevice);
		createRenderPass();
		mSwapChain->createFrameBuffers(mRenderPass);
		
		// 3 Start render
		// 3.2 Uniform
		mUniformManager = UniformManager::create();
		mUniformManager->init(mDevice, mCommandPool, mSwapChain->getImageCount());

		// 3.3 Pipeline
		mModel = Model::create(mDevice);
		mModel->loadModel("assets/shuttle/shuttle.obj", mDevice);

		mPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
		createPipeline();
		mSkyboxPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
		createSkyboxPipeline();

		// 3.4 Command buffer
		mCommandBuffers.resize(mSwapChain->getImageCount());
		createCommandBuffers();

		// 3.5 Sync
		createSyncObjects();

	}

	void Application::applyCommonPipelineState(const Wrapper::Pipeline::Ptr& pipeline, bool enableDepthWrite, VkCullModeFlagBits cullMode) {
		
		// Viewport & Scissor
		VkViewport viewport = { 0.0f, (float)mHeight, (float)mWidth, -(float)mHeight, 0.0f, 1.0f };
		VkRect2D scissor = { {0, 0}, {mWidth, mHeight} };
		pipeline->setViewports({ viewport });
		pipeline->setScissors({ scissor });

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
		pipeline->mDepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

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

		// Layout
		pipeline->mLayoutState.setLayoutCount = 1;
		auto descriptorSetLayoutPtr = mUniformManager->getDescriptorLayout();
		pipeline->mLayoutState.pSetLayouts = &descriptorSetLayoutPtr->getLayout();
		pipeline->mLayoutState.pushConstantRangeCount = 0;
		pipeline->mLayoutState.pPushConstantRanges = nullptr;
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
		applyCommonPipelineState(mSkyboxPipeline, false, VK_CULL_MODE_FRONT_BIT);

		// Render pass
		mSkyboxPipeline->build();
	}

	void Application::createPipeline() {

		// 2 Create shaders
		std::vector<Wrapper::Shader::Ptr> shaderGroup{};
		auto shaderVertex = Wrapper::Shader::create(mDevice, "shaders/vs.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
		shaderGroup.push_back(shaderVertex);
		auto shaderFragment = Wrapper::Shader::create(mDevice, "shaders/fs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");
		shaderGroup.push_back(shaderFragment);
		mPipeline->setShaderGroup(shaderGroup);

		// 3 Vertex stage 
		auto vertexBindingDes = mModel->getVertexInputBindingDescriptions();
		auto attributeDes = mModel->getAttributeDescriptions();

		mPipeline->mVertexInputState.vertexBindingDescriptionCount = vertexBindingDes.size();
		mPipeline->mVertexInputState.pVertexBindingDescriptions = vertexBindingDes.data();
		mPipeline->mVertexInputState.vertexAttributeDescriptionCount = attributeDes.size();
		mPipeline->mVertexInputState.pVertexAttributeDescriptions = attributeDes.data();

		// 4 Assembly stage
		mPipeline->mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mPipeline->mAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		mPipeline->mAssemblyState.primitiveRestartEnable = VK_FALSE;

		applyCommonPipelineState(mPipeline, true, VK_CULL_MODE_BACK_BIT);

		mPipeline->build();
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

		for (int i = 0; i < mSwapChain->getImageCount(); ++i) {

			mCommandBuffers[i] = Wrapper::CommandBuffer::create(mDevice, mCommandPool);
			mCommandBuffers[i]->begin();

			VkRenderPassBeginInfo renderBeginInfo{};
			renderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderBeginInfo.renderPass = mRenderPass->getRenderPass();
			renderBeginInfo.framebuffer = mSwapChain->getFrameBuffer(i);
			renderBeginInfo.renderArea.offset = { 0, 0 };
			renderBeginInfo.renderArea.extent = mSwapChain->getExtent();

			// Clear color
			std::vector< VkClearValue> clearColors{};
			VkClearValue finalClearColor{};
			finalClearColor.color = { 0.09f, 0.125f, 0.192f, 1.0f };
			clearColors.push_back(finalClearColor);

			VkClearValue mutiClearColor{};
			mutiClearColor.color = { 0.09f, 0.125f, 0.192f, 1.0f };
			clearColors.push_back(mutiClearColor);

			VkClearValue depthClearColor{};
			depthClearColor.depthStencil = { 1.0f, 0 };
			clearColors.push_back(depthClearColor);

			renderBeginInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
			renderBeginInfo.pClearValues = clearColors.data();

			mCommandBuffers[i]->beginRenderPass(renderBeginInfo);

			mCommandBuffers[i]->bindGraphicPipeline(mSkyboxPipeline->getPipeline());
			mCommandBuffers[i]->bindDescriptorSet(mSkyboxPipeline->getLayout(), mUniformManager->getDescriptorSet(mCurrentFrame));
			mCommandBuffers[i]->draw(36);

			mCommandBuffers[i]->bindGraphicPipeline(mPipeline->getPipeline());
			mCommandBuffers[i]->bindDescriptorSet(mPipeline->getLayout(), mUniformManager->getDescriptorSet(mCurrentFrame));
			mCommandBuffers[i]->bindVertexBuffer(mModel->getVertexBuffers());
			mCommandBuffers[i]->bindIndexBuffer(mModel->getIndexBuffer()->getBuffer());
			mCommandBuffers[i]->drawIndex(mModel->getIndexCount());

			mCommandBuffers[i]->endRenderPass();
			mCommandBuffers[i]->end();
		}
	}

	void Application::createSyncObjects() {
		for (int i = 0; i < mSwapChain->getImageCount(); ++i) {
			auto imageSemaphore = Wrapper::Semaphore::create(mDevice);
			mImageAvailableSemaphores.push_back(imageSemaphore);

			auto renderSemaphore = Wrapper::Semaphore::create(mDevice);
			mRenderFinishedSemaphores.push_back(renderSemaphore);

			auto fence = Wrapper::Fence::create(mDevice);
			mFences.push_back(fence);
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

		// Render pass
		mRenderPass = Wrapper::RenderPass::create(mDevice);
		createRenderPass();

		mSwapChain->createFrameBuffers(mRenderPass);

		// 3 Start render
		// 3.1 Pipeline
		mPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
		mSkyboxPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
		createPipeline();
		createSkyboxPipeline();

		// 3.2 Command buffer
		mCommandBuffers.resize(mSwapChain->getImageCount());
		createCommandBuffers();

		// 3.3 Sync
		createSyncObjects();
	}

	void Application::cleanupSwapChain() {
		
		mSwapChain.reset();
		mCommandBuffers.clear();
		mPipeline.reset();
		mRenderPass.reset();

		mImageAvailableSemaphores.clear();
		mRenderFinishedSemaphores.clear();
		mFences.clear();
	}

	void Application::mainLoop() {

		while (!mWindow->shouldClose()) {

			mWindow->pollEvents();
			mWindow->processEvent();

			mModel->update();

			mVPMatrices.mViewMatrix = mCamera.getViewMatrix();
			mVPMatrices.mProjectionMatrix = mCamera.getProjectMatrix();
			mUniformManager->update(mVPMatrices, mModel->getUniform(), mCurrentFrame);

			render();

		}

		vkDeviceWaitIdle(mDevice->getDevice());
	}

	void Application::render() {

		mFences[mCurrentFrame]->block();

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
		auto commandBuffer = mCommandBuffers[imageIndex]->getCommandBuffer();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		// 2.3 Signal semaphore
		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame]->getSemaphore() };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		// 2.4 Submit render command
		mFences[mCurrentFrame]->resetFence();
		if (vkQueueSubmit(mDevice->getGraphicQueue(), 1, &submitInfo, mFences[mCurrentFrame]->getFence()) != VK_SUCCESS) {

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

		mCurrentFrame = (mCurrentFrame + 1) % mSwapChain->getImageCount();
	}

	void Application::cleanUp() {

		mSkyboxPipeline.reset();
		mPipeline.reset();
		mRenderPass.reset();
		mSwapChain.reset();
		mDevice.reset();
		mSurface.reset();
		mInstance.reset();
		mWindow.reset();

	}
}