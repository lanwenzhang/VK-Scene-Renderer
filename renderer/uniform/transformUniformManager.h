#pragma once

#include "../../common.h"
#include "../../wrapper/device.h"
#include "../../wrapper/buffer.h"
#include "../../wrapper/description.h"
#include "../../wrapper/descriptorSet.h"
#include "../../loader/scene.h"

namespace LZ::Renderer {

    class TransformUniformManager {
    public:
        using Ptr = std::shared_ptr<TransformUniformManager>;
        static Ptr create() { return std::make_shared<TransformUniformManager>(); }

        TransformUniformManager();
        ~TransformUniformManager();

        void init(const LZ::Wrapper::Device::Ptr& device, size_t transformCount, const glm::mat4* initialData, int frameCount);
        std::vector<LZ::Wrapper::UniformParameter::Ptr> getParams() const;

    private:

        LZ::Wrapper::Device::Ptr mDevice{ nullptr };
        LZ::Wrapper::UniformParameter::Ptr mTransformParam{ nullptr };
    };

}