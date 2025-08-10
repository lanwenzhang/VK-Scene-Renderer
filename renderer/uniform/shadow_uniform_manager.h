#pragma once

#include "../../wrapper/device.h"
#include "../../wrapper/image.h"
#include "../../wrapper/description.h"
#include "../../wrapper/descriptor_set.h"

namespace lzvk::renderer {

    class ShadowUniformManager {
    public:

        using Ptr = std::shared_ptr<ShadowUniformManager>;
        static Ptr create() { return std::make_shared<ShadowUniformManager>(); }

        ShadowUniformManager();
        ~ShadowUniformManager();

        void init(const wrapper::Device::Ptr& device);
        void update(
            const wrapper::DescriptorSet::Ptr& descriptorSet,
            const renderer::Texture::Ptr& shadowTexture,
            int frameCount
        );

        std::vector<wrapper::UniformParameter::Ptr> getParams() const;

    private:

        lzvk::wrapper::Device::Ptr mDevice{ nullptr };
        lzvk::wrapper::UniformParameter::Ptr mShadowParam{ nullptr };
    };

}
