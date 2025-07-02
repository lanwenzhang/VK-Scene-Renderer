#include "scene_texture_manager.h"

namespace lzvk::renderer{

    SceneTextureManager::SceneTextureManager() {}
    SceneTextureManager::~SceneTextureManager() {}

    void SceneTextureManager::init(
        const lzvk::wrapper::Device::Ptr& device,
        const lzvk::wrapper::CommandPool::Ptr& commandPool,
        const SceneMeshRenderer::Ptr& sceneMesh,
        int frameCount)
    {
        mDevice = device;
        mCommandPool = commandPool;

        auto loadTextureParam = [&](const std::vector<Texture::Ptr>& textures,
            uint32_t binding,
            uint32_t fixedSize) -> lzvk::wrapper::UniformParameter::Ptr
            {
                auto param = lzvk::wrapper::UniformParameter::create();
                param->mBinding = binding;
                param->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                param->mStage = VK_SHADER_STAGE_FRAGMENT_BIT;
                param->mCount = fixedSize;

                param->mTextures.resize(fixedSize, nullptr);
                param->mImageInfos.resize(fixedSize);

                for (size_t i = 0; i < fixedSize; ++i)
                {
                    if (i < textures.size() && textures[i])
                    {
                        param->mTextures[i] = textures[i];
                        param->mImageInfos[i].imageView = textures[i]->getImageView();
                        param->mImageInfos[i].sampler = textures[i]->getSampler();
                        param->mImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    }
                    else
                    {
                        param->mImageInfos[i].imageView = VK_NULL_HANDLE;
                        param->mImageInfos[i].sampler = VK_NULL_HANDLE;
                        param->mImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    }
                }

                return param;
            };

        //mSceneEmissiveTexturesParam = loadTextureParam(sceneMesh->getEmissiveTextures(), 0, 1);
        //mSceneOcclusionTexturesParam = loadTextureParam(sceneMesh->getOcclusionTextures(), 0, 2);
        mSceneDiffuseTexturesParam = loadTextureParam(sceneMesh->getDiffuseTextures(), 0, static_cast<uint32_t>(sceneMesh->getDiffuseTextures().size()));
    }

    std::vector<lzvk::wrapper::UniformParameter::Ptr> SceneTextureManager::getDiffuseParams() const
    {
        return mSceneDiffuseTexturesParam ? std::vector{ mSceneDiffuseTexturesParam } : std::vector<lzvk::wrapper::UniformParameter::Ptr>{};
    }

    std::vector<lzvk::wrapper::UniformParameter::Ptr> SceneTextureManager::getEmissiveParams() const
    {
        return mSceneEmissiveTexturesParam ? std::vector{ mSceneEmissiveTexturesParam } : std::vector<lzvk::wrapper::UniformParameter::Ptr>{};
    }

    std::vector<lzvk::wrapper::UniformParameter::Ptr> SceneTextureManager::getOcclusionParams() const
    {
        return mSceneOcclusionTexturesParam ? std::vector{ mSceneOcclusionTexturesParam } : std::vector<lzvk::wrapper::UniformParameter::Ptr>{};
    }



}
