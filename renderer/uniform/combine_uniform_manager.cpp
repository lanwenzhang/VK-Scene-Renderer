#include "combine_uniform_manager.h"
#include "../texture/texture.h"

namespace lzvk::renderer {

    CombineUniformManager::CombineUniformManager(){}
    CombineUniformManager::~CombineUniformManager(){}


    void CombineUniformManager::init(const wrapper::Device::Ptr& device) {
        mDevice = device;

        mColorParam = wrapper::UniformParameter::create();
        mColorParam->mBinding = 0;
        mColorParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mColorParam->mStage = VK_SHADER_STAGE_FRAGMENT_BIT;
        mColorParam->mCount = 1;

        mAOParam = wrapper::UniformParameter::create();
        mAOParam->mBinding = 1;
        mAOParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mAOParam->mStage = VK_SHADER_STAGE_FRAGMENT_BIT;
        mAOParam->mCount = 1;
    }


    void CombineUniformManager::update(
        const wrapper::DescriptorSet::Ptr& descriptorSet,
        const renderer::Texture::Ptr& colorTexture,
        const renderer::Texture::Ptr& aoTexture,
        int frameCount
    ) {

        // mColor Param 
        mColorParam->mImageInfos.resize(1);
        mColorParam->mImageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        mColorParam->mImageInfos[0].imageView = colorTexture->getImageView();
        mColorParam->mImageInfos[0].sampler = colorTexture->getSampler();

        // mAO Param 
        mAOParam->mImageInfos.resize(1);
        mAOParam->mImageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        mAOParam->mImageInfos[0].imageView = aoTexture->getImageView();
        mAOParam->mImageInfos[0].sampler = aoTexture->getSampler();

        descriptorSet->updateImage(descriptorSet->getDescriptorSet(frameCount), mColorParam->mBinding, mColorParam->mImageInfos[0]);
        descriptorSet->updateImage(descriptorSet->getDescriptorSet(frameCount), mAOParam->mBinding, mAOParam->mImageInfos[0]);
    }

    std::vector<wrapper::UniformParameter::Ptr> CombineUniformManager::getParams() const {
       
        return { mColorParam , mAOParam };
    }

}
