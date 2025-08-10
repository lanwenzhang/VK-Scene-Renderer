#include "skybox_uniform_manager.h"
#include "../texture/cube_map_texture.h"

namespace lzvk::renderer {

    SkyboxUniformManager::SkyboxUniformManager(){}
    SkyboxUniformManager::~SkyboxUniformManager(){}

    void SkyboxUniformManager::init(const lzvk::wrapper::Device::Ptr& device) {
        
        mDevice = device;

        mSkyboxParam = lzvk::wrapper::UniformParameter::create();
        mSkyboxParam->mBinding = 3;
        mSkyboxParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mSkyboxParam->mStage = VK_SHADER_STAGE_FRAGMENT_BIT;
        mSkyboxParam->mCount = 1;

        mIrradianceParam = lzvk::wrapper::UniformParameter::create();
        mIrradianceParam->mBinding = 4;
        mIrradianceParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mIrradianceParam->mStage = VK_SHADER_STAGE_FRAGMENT_BIT;
        mIrradianceParam->mCount = 1;
    }

    void SkyboxUniformManager::updateCubeMap(
        const wrapper::DescriptorSet::Ptr& descriptorSet,
        const renderer::CubeMapTexture::Ptr& cubeMap,
        int frameCount
    ) {
         // mColor Param 
        mSkyboxParam->mImageInfos.resize(1);
        mSkyboxParam->mImageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        mSkyboxParam->mImageInfos[0].imageView = cubeMap->getImageView();
        mSkyboxParam->mImageInfos[0].sampler = cubeMap->getSampler();

        descriptorSet->updateImage(descriptorSet->getDescriptorSet(frameCount), mSkyboxParam->mBinding, mSkyboxParam->mImageInfos[0]);
    }


    void SkyboxUniformManager::updateIrradianceMap(
        const wrapper::DescriptorSet::Ptr& descriptorSet,
        const renderer::CubeMapTexture::Ptr& irradianceMap,
        int frameCount
    ) {
        mIrradianceParam->mImageInfos.resize(1);
        mIrradianceParam->mImageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        mIrradianceParam->mImageInfos[0].imageView = irradianceMap->getImageView();
        mIrradianceParam->mImageInfos[0].sampler = irradianceMap->getSampler();

        descriptorSet->updateImage(descriptorSet->getDescriptorSet(frameCount), mIrradianceParam->mBinding, mIrradianceParam->mImageInfos[0]);
    }

    std::vector<lzvk::wrapper::UniformParameter::Ptr> SkyboxUniformManager::getParams() const {
        return { mSkyboxParam, mIrradianceParam};
    }

}