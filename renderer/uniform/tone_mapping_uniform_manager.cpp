#include "tone_mapping_uniform_manager.h"
#include "../texture/texture.h"

namespace lzvk::renderer {

    ToneMappingUniformManager::ToneMappingUniformManager() {}
    ToneMappingUniformManager::~ToneMappingUniformManager() {}


    void ToneMappingUniformManager::init(const wrapper::Device::Ptr& device) {
        mDevice = device;

        mColorParam = wrapper::UniformParameter::create();
        mColorParam->mBinding = 0;
        mColorParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mColorParam->mStage = VK_SHADER_STAGE_FRAGMENT_BIT;
        mColorParam->mCount = 1;

    }


    void ToneMappingUniformManager::update(
        const wrapper::DescriptorSet::Ptr& descriptorSet,
        const renderer::Texture::Ptr& hdrTexture,
        int frameCount
    ) {

        // mColor Param 
        mColorParam->mImageInfos.resize(1);
        mColorParam->mImageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        mColorParam->mImageInfos[0].imageView = hdrTexture->getImageView();
        mColorParam->mImageInfos[0].sampler = hdrTexture->getSampler();

        descriptorSet->updateImage(descriptorSet->getDescriptorSet(frameCount), mColorParam->mBinding, mColorParam->mImageInfos[0]);
    }

    std::vector<wrapper::UniformParameter::Ptr> ToneMappingUniformManager::getParams() const {

        return { mColorParam};
    }

}
