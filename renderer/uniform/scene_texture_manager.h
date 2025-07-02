#pragma once

#include "../../common.h"
#include "../../wrapper/device.h"
#include "../../wrapper/buffer.h"
#include "../../wrapper/description.h"
#include "../scene/scene_mesh_renderer.h"

namespace lzvk::renderer {

    class SceneTextureManager {
    public:
        using Ptr = std::shared_ptr<SceneTextureManager>;
        static Ptr create() { return std::make_shared<SceneTextureManager>(); }

        SceneTextureManager();
        ~SceneTextureManager();

        void init(const lzvk::wrapper::Device::Ptr& device,
            const lzvk::wrapper::CommandPool::Ptr& commandPool,
            const SceneMeshRenderer::Ptr& sceneMesh,
            int frameCount);

        std::vector<lzvk::wrapper::UniformParameter::Ptr> getDiffuseParams() const;
        std::vector<lzvk::wrapper::UniformParameter::Ptr> getEmissiveParams() const;
        std::vector<lzvk::wrapper::UniformParameter::Ptr> getOcclusionParams() const;

    private:

        lzvk::wrapper::Device::Ptr mDevice;
        lzvk::wrapper::CommandPool::Ptr mCommandPool;

        lzvk::wrapper::UniformParameter::Ptr mSceneDiffuseTexturesParam;
        lzvk::wrapper::UniformParameter::Ptr mSceneEmissiveTexturesParam;
        lzvk::wrapper::UniformParameter::Ptr mSceneOcclusionTexturesParam;
    };
}