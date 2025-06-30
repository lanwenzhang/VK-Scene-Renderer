#include "skyboxUniformManager.h"
#include "../texture/cubeMapTexture.h"

namespace LZ::Renderer {

    SkyboxUniformManager::SkyboxUniformManager(){}
    SkyboxUniformManager::~SkyboxUniformManager(){}

    void SkyboxUniformManager::init(const LZ::Wrapper::Device::Ptr& device, const LZ::Wrapper::CommandPool::Ptr& commandPool, const std::string& path) {
        
        mDevice = device;
        mCommandPool = commandPool;

        mSkyboxParam = LZ::Wrapper::UniformParameter::create();
        mSkyboxParam->mBinding = 3;
        mSkyboxParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mSkyboxParam->mStage = VK_SHADER_STAGE_FRAGMENT_BIT;
        mSkyboxParam->mCount = 1;

        auto cubeMap = CubeMapTexture::create(mDevice, mCommandPool, "assets/piazza_bologni_1k.hdr");
        mSkyboxParam->mCubeMap = cubeMap;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageView = cubeMap->getImageView();
        imageInfo.sampler = cubeMap->getSampler();
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        mSkyboxParam->mImageInfos.push_back(imageInfo);
    }

    std::vector<LZ::Wrapper::UniformParameter::Ptr> SkyboxUniformManager::getParams() const {
        return { mSkyboxParam };
    }

}