#include "blur_uniform_manager.h"

namespace lzvk::renderer {

    BlurUniformManager::BlurUniformManager() {}
    BlurUniformManager::~BlurUniformManager() {}

    void BlurUniformManager::init(const lzvk::wrapper::Device::Ptr& device) {
        
        mDevice = device;

        mDepthParam = lzvk::wrapper::UniformParameter::create();
        mDepthParam->mBinding = 0;
        mDepthParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mDepthParam->mStage = VK_SHADER_STAGE_COMPUTE_BIT;
        mDepthParam->mCount = 1;

        mInputAOParam = lzvk::wrapper::UniformParameter::create();
        mInputAOParam->mBinding = 1;
        mInputAOParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mInputAOParam->mStage = VK_SHADER_STAGE_COMPUTE_BIT;
        mInputAOParam->mCount = 1;

        mOutputAOParam = lzvk::wrapper::UniformParameter::create();
        mOutputAOParam->mBinding = 2;
        mOutputAOParam->mDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        mOutputAOParam->mStage = VK_SHADER_STAGE_COMPUTE_BIT;
        mOutputAOParam->mCount = 1;
    }

    std::vector<lzvk::wrapper::UniformParameter::Ptr> BlurUniformManager::getParams() const {
        return { mDepthParam, mInputAOParam, mOutputAOParam };
    }

    void BlurUniformManager::update(
        const lzvk::wrapper::DescriptorSet::Ptr& descriptorSet,
        const lzvk::renderer::Texture::Ptr& depthTexture,
        const lzvk::renderer::Texture::Ptr& inputTexture,
        const lzvk::wrapper::Image::Ptr& outputAO,
        int frameCount)
    {

        // 1. Depth texture (sampler + view)
        mDepthParam->mImageInfos.resize(1);
        mDepthParam->mImageInfos[0].sampler = depthTexture->getSampler();
        mDepthParam->mImageInfos[0].imageView = depthTexture->getImageView();
        mDepthParam->mImageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // 2. Input AO
        mInputAOParam->mImageInfos.resize(1);
        mInputAOParam->mImageInfos[0].sampler = inputTexture->getSampler();
        mInputAOParam->mImageInfos[0].imageView = inputTexture->getImageView();
        mInputAOParam->mImageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // 3. Output AO (storage image)
        mOutputAOParam->mImageInfos.resize(1);
        mOutputAOParam->mImageInfos[0].sampler = VK_NULL_HANDLE;
        mOutputAOParam->mImageInfos[0].imageView = outputAO->getImageView();
        mOutputAOParam->mImageInfos[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        descriptorSet->updateImage(descriptorSet->getDescriptorSet(frameCount), mDepthParam->mBinding, mDepthParam->mImageInfos[0]);
        descriptorSet->updateImage(descriptorSet->getDescriptorSet(frameCount), mInputAOParam->mBinding, mInputAOParam->mImageInfos[0]);
        descriptorSet->updateStorageImage(descriptorSet->getDescriptorSet(frameCount), mOutputAOParam->mBinding, mOutputAOParam->mImageInfos[0]);
    }

}
