#pragma once

#include "../../common.h"
#include "../../wrapper/device.h"
#include "../../wrapper/buffer.h"
#include "../../wrapper/description.h"

namespace LZ::Renderer {

    class SkyboxUniformManager {
    public:

        using Ptr = std::shared_ptr<SkyboxUniformManager>;
        static Ptr create() { return std::make_shared<SkyboxUniformManager>(); }

        SkyboxUniformManager();
        ~SkyboxUniformManager();

        void init(const LZ::Wrapper::Device::Ptr& device, const LZ::Wrapper::CommandPool::Ptr& commandPool, const std::string& path);
        std::vector<LZ::Wrapper::UniformParameter::Ptr> getParams() const;

    private:

        LZ::Wrapper::Device::Ptr mDevice{ nullptr };
        LZ::Wrapper::CommandPool::Ptr mCommandPool{ nullptr };
        LZ::Wrapper::UniformParameter::Ptr mSkyboxParam{nullptr};
    };

}