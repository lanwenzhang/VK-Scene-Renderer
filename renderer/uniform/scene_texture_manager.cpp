#include "scene_texture_manager.h"

namespace lzvk::renderer{

    SceneTextureManager::SceneTextureManager() {}
    SceneTextureManager::~SceneTextureManager() {}

    void SceneTextureManager::init(
        const lzvk::wrapper::Device::Ptr& device,
        const lzvk::wrapper::CommandPool::Ptr& commandPool,
        const lzvk::loader::MeshData& meshData,
        int frameCount)
    {
        mDevice = device;
        mCommandPool = commandPool;

        //
        // Load diffuse textures
        //
        std::vector<Texture::Ptr> diffuseTextures;
        for (const auto& texPath : meshData.diffuseTextureFiles) {
          /*  std::cout << "[SceneTextureManager] Loading diffuse texture: " << texPath << std::endl;*/
            auto tex = std::make_shared<Texture>(
                mDevice,
                mCommandPool,
                texPath
            );
            diffuseTextures.push_back(tex);
        }

        mSceneDiffuseTexturesParam = loadTextureParam(
            diffuseTextures,
            0,
            static_cast<uint32_t>(diffuseTextures.size())
        );


        // Load emissive textures
        std::vector<Texture::Ptr> emissiveTextures;
        for (const auto& texPath : meshData.emissiveTextureFiles) {
       /*     std::cout << "[SceneTextureManager] Loading emissive texture: " << texPath << std::endl;*/
            auto tex = std::make_shared<Texture>(
                mDevice,
                mCommandPool,
                texPath
            );
            emissiveTextures.push_back(tex);
        }

        mSceneEmissiveTexturesParam = loadTextureParam(
            emissiveTextures,
            0,
            static_cast<uint32_t>(emissiveTextures.size())
        );


        // Load normal textures
        std::vector<Texture::Ptr> normalTextures;
        for (const auto& texPath : meshData.normalTextureFiles) {
       /*     std::cout << "[SceneTextureManager] Loading normal texture: " << texPath << std::endl;*/
            auto tex = std::make_shared<Texture>(
                mDevice,
                mCommandPool,
                texPath
            );
            normalTextures.push_back(tex);
        }

        mSceneNormalTexturesParam = loadTextureParam(
            normalTextures,
            0,
            static_cast<uint32_t>(normalTextures.size())
        );

        // Load opacity textures
        std::vector<Texture::Ptr> opacityTextures;
        for (const auto& texPath : meshData.opacityTextureFiles) {
            /*     std::cout << "[SceneTextureManager] Loading normal texture: " << texPath << std::endl;*/
            auto tex = std::make_shared<Texture>(
                mDevice,
                mCommandPool,
                texPath
            );
            opacityTextures.push_back(tex);
        }

        mSceneOpacityTexturesParam = loadTextureParam(
            opacityTextures,
            0,
            static_cast<uint32_t>(opacityTextures.size())
        );

    }

    lzvk::wrapper::UniformParameter::Ptr SceneTextureManager::loadTextureParam(
        const std::vector<lzvk::renderer::Texture::Ptr>& textures,
        uint32_t binding,
        uint32_t fixedSize)
    {
        auto param = lzvk::wrapper::UniformParameter::create();
        param->mBinding = binding;
        param->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        param->mStage = VK_SHADER_STAGE_FRAGMENT_BIT;
        param->mCount = fixedSize;

        param->mTextures.resize(fixedSize, nullptr);
        param->mImageInfos.resize(fixedSize);

        for (size_t i = 0; i < fixedSize; ++i) {
            if (i < textures.size() && textures[i]) {
                param->mTextures[i] = textures[i];
                param->mImageInfos[i] = textures[i]->getImageInfo();
            }
            else {
                param->mImageInfos[i].imageView = VK_NULL_HANDLE;
                param->mImageInfos[i].sampler = VK_NULL_HANDLE;
                param->mImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
        }
        return param;
    }

}
