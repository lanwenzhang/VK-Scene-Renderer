#pragma once

#include "../../common.h"
#include "../../core/type.h"
#include "../../wrapper/device.h"
#include "../../wrapper/image.h"
#include "../../wrapper/description.h"
#include "../../wrapper/descriptor_set.h"
#include "../../renderer/texture/texture.h"

namespace lzvk::renderer {

    class SSAOUniformManager {
    public:
        using Ptr = std::shared_ptr<SSAOUniformManager>;
        static Ptr create() { return std::make_shared<SSAOUniformManager>(); }

        SSAOUniformManager();
        ~SSAOUniformManager();

        void init(const lzvk::wrapper::Device::Ptr& device);

        std::vector<lzvk::wrapper::UniformParameter::Ptr> getParams() const;

        void update(
            const lzvk::wrapper::DescriptorSet::Ptr& descriptorSet,
            const lzvk::renderer::Texture::Ptr& depthTexture,
            const lzvk::renderer::Texture::Ptr& rotationTexture,
            const lzvk::wrapper::Image::Ptr& outputImage,
            int frameCount);

    private:
        lzvk::wrapper::Device::Ptr mDevice{ nullptr };

        lzvk::wrapper::UniformParameter::Ptr mDepthParam{ nullptr };
        lzvk::wrapper::UniformParameter::Ptr mRotationParam{ nullptr };
        lzvk::wrapper::UniformParameter::Ptr mOutputParam{ nullptr };
    };

}
