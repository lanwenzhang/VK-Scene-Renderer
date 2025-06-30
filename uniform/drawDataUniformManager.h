#pragma once

#include "../base.h"
#include "../vulkanWrapper/device.h"
#include "../vulkanWrapper/buffer.h"
#include "../vulkanWrapper/description.h"
#include "../vulkanWrapper/descriptorSet.h"
#include "../scene/scene.h"

namespace FF {

    class DrawDataUniformManager {
    public:

        using Ptr = std::shared_ptr<DrawDataUniformManager>;
        static Ptr create() { return std::make_shared<DrawDataUniformManager>(); }

        DrawDataUniformManager();
        ~DrawDataUniformManager();

        void init(const Wrapper::Device::Ptr& device,
            size_t drawCount,
            const DrawData* initialData,
            int frameCount);

        std::vector<Wrapper::UniformParameter::Ptr> getParams() const;

    private:
        Wrapper::Device::Ptr mDevice{ nullptr };
        Wrapper::UniformParameter::Ptr mDrawDataParam{ nullptr };
    };
}
