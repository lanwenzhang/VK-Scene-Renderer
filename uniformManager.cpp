#include "uniformManager.h"

UniformManager::UniformManager() {

}

UniformManager::~UniformManager() {

}

void UniformManager::init(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, int frameCount) {

	mDevice = device;

	// 1 View projection matrix
	// 1.1 Uniform parameter - vp matrix
	auto vpParam = Wrapper::UniformParameter::create();
	vpParam->mBinding = 0;
	vpParam->mCount = 1;
	vpParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vpParam->mSize = sizeof(VPMatrices);
	vpParam->mStage = VK_SHADER_STAGE_VERTEX_BIT;

	// 1.2 Create uniform buffer
	for (int i = 0; i < frameCount; ++i) {

		auto buffer = Wrapper::Buffer::createUniformBuffer(device, vpParam->mSize, nullptr);
		vpParam->mBuffers.push_back(buffer);
	}

	mUniformParams.push_back(vpParam);

	// 2 Model matrix
	// 2.1 Uniform parameter - model matrix
	auto objectParam = Wrapper::UniformParameter::create();
	objectParam->mBinding = 1;
	objectParam->mCount = 1;
	objectParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	objectParam->mSize = sizeof(ObjectUniform);
	objectParam->mStage = VK_SHADER_STAGE_VERTEX_BIT;

	// 2.2 Create uniform buffer
	for (int i = 0; i < frameCount; ++i) {

		auto buffer = Wrapper::Buffer::createUniformBuffer(device, objectParam->mSize, nullptr);
		objectParam->mBuffers.push_back(buffer);
	}

	mUniformParams.push_back(objectParam);

	// 2.3 Create texture uniform
	auto textureParam = Wrapper::UniformParameter::create();
	textureParam->mBinding = 2;
	textureParam->mCount = 1;
	textureParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureParam->mStage = VK_SHADER_STAGE_FRAGMENT_BIT;
	textureParam->mTexture = Texture::create(mDevice, commandPool, "assets/shuttle.jpg");

	mUniformParams.push_back(textureParam);

	// 3 Descriptor layout
	mDescriptorSetLayout = Wrapper::DescriptorSetLayout::create(device);
	mDescriptorSetLayout->build(mUniformParams);

	// 4 Descriptor pool
	mDescriptorPool = Wrapper::DescriptorPool::create(device);
	mDescriptorPool->build(mUniformParams, frameCount);

	// 5 Descriptor set
	mDescriptorSet = Wrapper::DescriptorSet::create(device, mUniformParams, mDescriptorSetLayout, mDescriptorPool, frameCount);

}

void UniformManager::update(const VPMatrices& vpMatrices, const ObjectUniform& objectUniform, const int& frameCount) {
	
	mUniformParams[0]->mBuffers[frameCount]->updateBufferByMap((void*)(&vpMatrices), sizeof(VPMatrices));

	mUniformParams[1]->mBuffers[frameCount]->updateBufferByMap((void*)(&objectUniform), sizeof(ObjectUniform));
}