#pragma once

#include "../../common.h"
#include "../../core/type.h"
#include "../../wrapper/device.h"
#include "../../wrapper/buffer.h"
#include "../../wrapper/description.h"
#include "../../wrapper/descriptor_set.h"

namespace lzvk::renderer {

    class FrameUniformManager {
    public:
        using Ptr = std::shared_ptr<FrameUniformManager>;
        static Ptr create() { return std::make_shared<FrameUniformManager>(); }

        FrameUniformManager();
        ~FrameUniformManager();

        void init(const lzvk::wrapper::Device::Ptr& device, int frameCount);
        void update(const glm::mat4& view, const glm::mat4& projection, const lzvk::wrapper::DescriptorSet::Ptr& descriptorSet, int frameIndex);

        std::vector<lzvk::wrapper::UniformParameter::Ptr> getParams() const;

    private:

        lzvk::wrapper::Device::Ptr mDevice{ nullptr };
        lzvk::wrapper::UniformParameter::Ptr mVpParam{ nullptr };
    };

}