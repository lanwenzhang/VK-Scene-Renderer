#pragma once

#include "../base.h"
#include "../vulkanWrapper/device.h"
#include "../vulkanWrapper/buffer.h"
#include "../vulkanWrapper/description.h"

namespace FF {

    class SkyboxUniformManager {
    public:

        using Ptr = std::shared_ptr<SkyboxUniformManager>;
        static Ptr create() { return std::make_shared<SkyboxUniformManager>(); }

        SkyboxUniformManager();
        ~SkyboxUniformManager();

        void init(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, const std::string& path);
        std::vector<Wrapper::UniformParameter::Ptr> getParams() const;

    private:

        Wrapper::Device::Ptr mDevice{ nullptr };
        Wrapper::CommandPool::Ptr mCommandPool{ nullptr };
        Wrapper::UniformParameter::Ptr mSkyboxParam{nullptr};
    };

}