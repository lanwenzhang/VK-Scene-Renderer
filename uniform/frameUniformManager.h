#pragma once

#include "../base.h"
#include "../vulkanWrapper/device.h"
#include "../vulkanWrapper/buffer.h"
#include "../vulkanWrapper/description.h"
#include "../vulkanWrapper/descriptorSet.h"

namespace FF {

    class FrameUniformManager {
    public:
        using Ptr = std::shared_ptr<FrameUniformManager>;
        static Ptr create() { return std::make_shared<FrameUniformManager>(); }

        FrameUniformManager();
        ~FrameUniformManager();

        void init(const Wrapper::Device::Ptr& device, int frameCount);
        void update(const glm::mat4& view, const glm::mat4& projection, const Wrapper::DescriptorSet::Ptr& descriptorSet, int frameIndex);

        std::vector<FF::Wrapper::UniformParameter::Ptr> getParams() const;

    private:

        Wrapper::Device::Ptr mDevice{ nullptr };
        Wrapper::UniformParameter::Ptr mVpParam{ nullptr };
    };

}