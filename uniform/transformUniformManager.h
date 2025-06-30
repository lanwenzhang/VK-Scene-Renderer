#pragma once

#include "../base.h"
#include "../vulkanWrapper/device.h"
#include "../vulkanWrapper/buffer.h"
#include "../vulkanWrapper/description.h"
#include "../vulkanWrapper/descriptorSet.h"
#include "../scene/scene.h"

namespace FF {

    class TransformUniformManager {
    public:
        using Ptr = std::shared_ptr<TransformUniformManager>;
        static Ptr create() { return std::make_shared<TransformUniformManager>(); }

        TransformUniformManager();
        ~TransformUniformManager();

        void init(const Wrapper::Device::Ptr& device, size_t transformCount, const glm::mat4* initialData, int frameCount);
        std::vector<Wrapper::UniformParameter::Ptr> getParams() const;

    private:

        Wrapper::Device::Ptr mDevice{ nullptr };
        Wrapper::UniformParameter::Ptr mTransformParam{ nullptr };
    };

}