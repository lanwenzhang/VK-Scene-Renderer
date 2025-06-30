#pragma once

#include "../../common.h"
#include "../../core/type.h"
#include "../../wrapper/device.h"
#include "../../wrapper/buffer.h"
#include "../../wrapper/description.h"
#include "../../wrapper/descriptorSet.h"

namespace LZ::Renderer {

    class FrameUniformManager {
    public:
        using Ptr = std::shared_ptr<FrameUniformManager>;
        static Ptr create() { return std::make_shared<FrameUniformManager>(); }

        FrameUniformManager();
        ~FrameUniformManager();

        void init(const LZ::Wrapper::Device::Ptr& device, int frameCount);
        void update(const glm::mat4& view, const glm::mat4& projection, const LZ::Wrapper::DescriptorSet::Ptr& descriptorSet, int frameIndex);

        std::vector<LZ::Wrapper::UniformParameter::Ptr> getParams() const;

    private:

        LZ::Wrapper::Device::Ptr mDevice{ nullptr };
        LZ::Wrapper::UniformParameter::Ptr mVpParam{ nullptr };
    };

}