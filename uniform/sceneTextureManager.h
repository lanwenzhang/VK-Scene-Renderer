#pragma once

#include "../base.h"
#include "../vulkanWrapper/device.h"
#include "../vulkanWrapper/buffer.h"
#include "../vulkanWrapper/description.h"
#include "../scene/sceneMeshRenderer.h"

namespace FF {

    class SceneTextureManager {
    public:
        using Ptr = std::shared_ptr<SceneTextureManager>;
        static Ptr create() { return std::make_shared<SceneTextureManager>(); }

        SceneTextureManager();
        ~SceneTextureManager();

        void init(const Wrapper::Device::Ptr& device,
            const Wrapper::CommandPool::Ptr& commandPool,
            const SceneMeshRenderer::Ptr& sceneMesh,
            int frameCount);

        std::vector<Wrapper::UniformParameter::Ptr> getDiffuseParams() const;
        std::vector<Wrapper::UniformParameter::Ptr> getEmissiveParams() const;
        std::vector<Wrapper::UniformParameter::Ptr> getOcclusionParams() const;

    private:

        Wrapper::Device::Ptr mDevice;
        Wrapper::CommandPool::Ptr mCommandPool;

        Wrapper::UniformParameter::Ptr mSceneDiffuseTexturesParam;
        Wrapper::UniformParameter::Ptr mSceneEmissiveTexturesParam;
        Wrapper::UniformParameter::Ptr mSceneOcclusionTexturesParam;
    };
}