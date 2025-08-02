#pragma once

#include "../../common.h"
#include "../../core/type.h"
#include "../../wrapper/device.h"
#include "../../wrapper/image.h"
#include "../../wrapper/description.h"
#include "../../wrapper/descriptor_set.h"
#include "../../renderer/texture/texture.h"

namespace lzvk::renderer {

    class BlurUniformManager {
    public:
        using Ptr = std::shared_ptr<BlurUniformManager>;
        static Ptr create() { return std::make_shared<BlurUniformManager>(); }

        BlurUniformManager();
        ~BlurUniformManager();

        void init(const lzvk::wrapper::Device::Ptr& device);

        std::vector<lzvk::wrapper::UniformParameter::Ptr> getParams() const;

        void update(
            const lzvk::wrapper::DescriptorSet::Ptr& descriptorSet,
            const lzvk::renderer::Texture::Ptr& depthTexture,
            const lzvk::renderer::Texture::Ptr& inputTexture,
            const lzvk::wrapper::Image::Ptr& outputAO,
            int frameCount);

    private:

        lzvk::wrapper::Device::Ptr mDevice{ nullptr };
        lzvk::wrapper::UniformParameter::Ptr mDepthParam{ nullptr };
        lzvk::wrapper::UniformParameter::Ptr mInputAOParam{ nullptr };
        lzvk::wrapper::UniformParameter::Ptr mOutputAOParam{ nullptr };
    };

}
