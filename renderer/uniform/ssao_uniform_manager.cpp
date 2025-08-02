#include "ssao_uniform_manager.h"

namespace lzvk::renderer {

    SSAOUniformManager::SSAOUniformManager() {}
    SSAOUniformManager::~SSAOUniformManager() {}

    void SSAOUniformManager::init(const lzvk::wrapper::Device::Ptr& device) {
        
        mDevice = device;

        mDepthParam = lzvk::wrapper::UniformParameter::create();
        mDepthParam->mBinding = 0;
        mDepthParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mDepthParam->mStage = VK_SHADER_STAGE_COMPUTE_BIT;
        mDepthParam->mCount = 1;

        mRotationParam = lzvk::wrapper::UniformParameter::create();
        mRotationParam->mBinding = 1;
        mRotationParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mRotationParam->mStage = VK_SHADER_STAGE_COMPUTE_BIT;
        mRotationParam->mCount = 1;

        mOutputParam = lzvk::wrapper::UniformParameter::create();
        mOutputParam->mBinding = 2;
        mOutputParam->mDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        mOutputParam->mStage = VK_SHADER_STAGE_COMPUTE_BIT;
        mOutputParam->mCount = 1;
    }

    std::vector<lzvk::wrapper::UniformParameter::Ptr> SSAOUniformManager::getParams() const {
        return { mDepthParam, mRotationParam, mOutputParam };
    }

    void SSAOUniformManager::update(
        const lzvk::wrapper::DescriptorSet::Ptr& descriptorSet,
        const lzvk::renderer::Texture::Ptr& depthTexture,
        const lzvk::renderer::Texture::Ptr& rotationTexture,
        const lzvk::wrapper::Image::Ptr& outputImage,
        int frameCount)
    {

   
        // 1. Depth texture
        mDepthParam->mImageInfos.resize(1);
        mDepthParam->mImageInfos[0].sampler = depthTexture->getSampler();
        mDepthParam->mImageInfos[0].imageView = depthTexture->getImageView();
        mDepthParam->mImageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // 2. Rotation texture
        mRotationParam->mImageInfos.resize(1);
        mRotationParam->mImageInfos[0].sampler = rotationTexture->getSampler();
        mRotationParam->mImageInfos[0].imageView = rotationTexture->getImageView();
        mRotationParam->mImageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // 3. Output SSAO image
        mOutputParam->mImageInfos.resize(1);
        mOutputParam->mImageInfos[0].sampler = VK_NULL_HANDLE;
        mOutputParam->mImageInfos[0].imageView = outputImage->getImageView();
        mOutputParam->mImageInfos[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        descriptorSet->updateImage(descriptorSet->getDescriptorSet(frameCount), mDepthParam->mBinding, mDepthParam->mImageInfos[0]);
        descriptorSet->updateImage(descriptorSet->getDescriptorSet(frameCount), mRotationParam->mBinding, mRotationParam->mImageInfos[0]);
        descriptorSet->updateStorageImage(descriptorSet->getDescriptorSet(frameCount), mOutputParam->mBinding, mOutputParam->mImageInfos[0]);
    }
}



