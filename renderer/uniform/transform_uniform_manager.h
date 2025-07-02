#pragma once

#include "../../common.h"
#include "../../wrapper/device.h"
#include "../../wrapper/buffer.h"
#include "../../wrapper/description.h"
#include "../../wrapper/descriptor_set.h"
#include "../../loader/scene.h"

namespace lzvk::renderer {

    class TransformUniformManager {
    public:
        using Ptr = std::shared_ptr<TransformUniformManager>;
        static Ptr create() { return std::make_shared<TransformUniformManager>(); }

        TransformUniformManager();
        ~TransformUniformManager();

        void init(const lzvk::wrapper::Device::Ptr& device, size_t transformCount, const glm::mat4* initialData, int frameCount);
        std::vector<lzvk::wrapper::UniformParameter::Ptr> getParams() const;

    private:

        lzvk::wrapper::Device::Ptr mDevice{ nullptr };
        lzvk::wrapper::UniformParameter::Ptr mTransformParam{ nullptr };
    };

}