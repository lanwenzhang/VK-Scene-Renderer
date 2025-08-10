#pragma once

#include "../../common.h"
#include "../../wrapper/device.h"
#include "../../wrapper/buffer.h"
#include "../../wrapper/description.h"
#include "../../wrapper/descriptor_set.h"
#include "../texture/cube_map_texture.h"

namespace lzvk::renderer {

    class SkyboxUniformManager {
    public:

        using Ptr = std::shared_ptr<SkyboxUniformManager>;
        static Ptr create() { return std::make_shared<SkyboxUniformManager>(); }

        SkyboxUniformManager();
        ~SkyboxUniformManager();

        void init(const lzvk::wrapper::Device::Ptr& device);
        void updateCubeMap(
            const wrapper::DescriptorSet::Ptr& descriptorSet,
            const renderer::CubeMapTexture::Ptr& cubeMap,
            int frameCount
        );


        void updateIrradianceMap(
            const wrapper::DescriptorSet::Ptr& descriptorSet,
            const renderer::CubeMapTexture::Ptr& irradianceMap,
            int frameCount
        );

        std::vector<lzvk::wrapper::UniformParameter::Ptr> getParams() const;


    private:

        lzvk::wrapper::Device::Ptr mDevice{ nullptr };
        lzvk::wrapper::CommandPool::Ptr mCommandPool{ nullptr };
        lzvk::wrapper::UniformParameter::Ptr mSkyboxParam{ nullptr };
        lzvk::wrapper::UniformParameter::Ptr mIrradianceParam{ nullptr };
    };

}