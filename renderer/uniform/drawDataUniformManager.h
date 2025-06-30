#pragma once

#include "../../common.h"
#include "../../wrapper/device.h"
#include "../../wrapper/buffer.h"
#include "../../wrapper/description.h"
#include "../../wrapper/descriptorSet.h"
#include "../../loader/scene.h"

namespace LZ::Renderer {

    class DrawDataUniformManager {
    public:

        using Ptr = std::shared_ptr<DrawDataUniformManager>;
        static Ptr create() { return std::make_shared<DrawDataUniformManager>(); }

        DrawDataUniformManager();
        ~DrawDataUniformManager();

        void init(const LZ::Wrapper::Device::Ptr& device,
            size_t drawCount,
            const LZ::Loader::DrawData* initialData,
            int frameCount);

        std::vector<LZ::Wrapper::UniformParameter::Ptr> getParams() const;

    private:
        
        LZ::Wrapper::Device::Ptr mDevice{ nullptr };
        LZ::Wrapper::UniformParameter::Ptr mDrawDataParam{ nullptr };
    };
}
