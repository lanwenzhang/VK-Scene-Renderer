#pragma once

#include "../../common.h"
#include "../../wrapper/device.h"
#include "../../wrapper/buffer.h"
#include "../../wrapper/description.h"
#include "../scene/sceneMeshRenderer.h"

namespace LZ::Renderer {

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

        std::vector<LZ::Wrapper::UniformParameter::Ptr> getDiffuseParams() const;
        std::vector<LZ::Wrapper::UniformParameter::Ptr> getEmissiveParams() const;
        std::vector<LZ::Wrapper::UniformParameter::Ptr> getOcclusionParams() const;

    private:

        LZ::Wrapper::Device::Ptr mDevice;
        LZ::Wrapper::CommandPool::Ptr mCommandPool;

        LZ::Wrapper::UniformParameter::Ptr mSceneDiffuseTexturesParam;
        LZ::Wrapper::UniformParameter::Ptr mSceneEmissiveTexturesParam;
        LZ::Wrapper::UniformParameter::Ptr mSceneOcclusionTexturesParam;
    };
}