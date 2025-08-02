#pragma once

#include "../../wrapper/device.h"
#include "../../wrapper/image.h"
#include "../../wrapper/description.h"
#include "../../wrapper/descriptor_set.h"

namespace lzvk::renderer {

    class CombineUniformManager {
    public:

        using Ptr = std::shared_ptr<CombineUniformManager>;
        static Ptr create() { return std::make_shared<CombineUniformManager>(); }

        CombineUniformManager();
        ~CombineUniformManager();

        void init(const wrapper::Device::Ptr& device);
        void update(
            const wrapper::DescriptorSet::Ptr& descriptorSet,
            const renderer::Texture::Ptr& colorTexture,
            const renderer::Texture::Ptr& aoTexture,
            int frameCount
        );

        std::vector<wrapper::UniformParameter::Ptr> getParams() const;

    private:

        lzvk::wrapper::Device::Ptr mDevice{ nullptr };
        lzvk::wrapper::UniformParameter::Ptr mColorParam{ nullptr };
        lzvk::wrapper::UniformParameter::Ptr mAOParam{ nullptr };
    };

}
