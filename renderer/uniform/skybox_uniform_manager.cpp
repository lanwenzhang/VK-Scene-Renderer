#include "skybox_uniform_manager.h"
#include "../texture/cube_map_texture.h"

namespace lzvk::renderer {

    SkyboxUniformManager::SkyboxUniformManager(){}
    SkyboxUniformManager::~SkyboxUniformManager(){}

    void SkyboxUniformManager::init(const lzvk::wrapper::Device::Ptr& device, const lzvk::wrapper::CommandPool::Ptr& commandPool, const std::string& path) {
        
        mDevice = device;
        mCommandPool = commandPool;

        mSkyboxParam = lzvk::wrapper::UniformParameter::create();
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

    std::vector<lzvk::wrapper::UniformParameter::Ptr> SkyboxUniformManager::getParams() const {
        return { mSkyboxParam };
    }

}