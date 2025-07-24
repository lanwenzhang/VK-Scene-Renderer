#pragma once

#include "../../common.h"
#include "../../loader/mesh.h"
#include "../texture/texture.h"
#include "../../wrapper/device.h"
#include "../../wrapper/buffer.h"
#include "../../wrapper/description.h"

namespace lzvk::renderer {

    class SceneTextureManager {
    public:
        using Ptr = std::shared_ptr<SceneTextureManager>;
        static Ptr create() { return std::make_shared<SceneTextureManager>(); }

        SceneTextureManager();
        ~SceneTextureManager();

        void init(const lzvk::wrapper::Device::Ptr& device,
            const lzvk::wrapper::CommandPool::Ptr& commandPool,
            const lzvk::loader::MeshData& meshData,
            int frameCount);

        lzvk::wrapper::UniformParameter::Ptr loadTextureParam(
            const std::vector<lzvk::renderer::Texture::Ptr>& textures,
            uint32_t binding,
            uint32_t fixedSize);

        [[nodiscard]] auto getDiffuseParams() const { return std::vector{ mSceneDiffuseTexturesParam }; }
        [[nodiscard]] auto getEmissiveParams() const { return std::vector{ mSceneEmissiveTexturesParam }; }
        [[nodiscard]] auto getNormalParams() const { return std::vector{ mSceneNormalTexturesParam }; }

    private:

        lzvk::wrapper::Device::Ptr mDevice;
        lzvk::wrapper::CommandPool::Ptr mCommandPool;

        lzvk::wrapper::UniformParameter::Ptr mSceneDiffuseTexturesParam;
        lzvk::wrapper::UniformParameter::Ptr mSceneEmissiveTexturesParam;
        lzvk::wrapper::UniformParameter::Ptr mSceneNormalTexturesParam;
    };
}