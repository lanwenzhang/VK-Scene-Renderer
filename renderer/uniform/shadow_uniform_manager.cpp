#include "shadow_uniform_manager.h"
#include "../texture/texture.h"

namespace lzvk::renderer {

    ShadowUniformManager::ShadowUniformManager() {}
    ShadowUniformManager::~ShadowUniformManager() {}


    void ShadowUniformManager::init(const wrapper::Device::Ptr& device) {
        mDevice = device;

        mShadowParam = wrapper::UniformParameter::create();
        mShadowParam->mBinding = 0;
        mShadowParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mShadowParam->mStage = VK_SHADER_STAGE_FRAGMENT_BIT;
        mShadowParam->mCount = 1;

    }


    void ShadowUniformManager::update(
        const wrapper::DescriptorSet::Ptr& descriptorSet,
        const renderer::Texture::Ptr& shadowTexture,
        int frameCount
    ) {

        // mColor Param 
        mShadowParam->mImageInfos.resize(1);
        mShadowParam->mImageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        mShadowParam->mImageInfos[0].imageView = shadowTexture->getImageView();
        mShadowParam->mImageInfos[0].sampler = shadowTexture->getSampler();

        descriptorSet->updateImage(descriptorSet->getDescriptorSet(frameCount), mShadowParam->mBinding, mShadowParam->mImageInfos[0]);
    }

    std::vector<wrapper::UniformParameter::Ptr> ShadowUniformManager::getParams() const {

        return { mShadowParam };
    }

}
