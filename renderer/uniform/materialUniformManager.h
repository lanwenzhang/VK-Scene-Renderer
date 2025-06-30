#pragma once

#include "../../common.h"
#include "../../wrapper/device.h"
#include "../../wrapper/buffer.h"
#include "../../wrapper/description.h"
#include "../../wrapper//descriptorSet.h"
#include "../../loader/mesh.h"

namespace LZ::Renderer {


    class MaterialUniformManager {
    public:
        using Ptr = std::shared_ptr<MaterialUniformManager>;
        static Ptr create() { return std::make_shared<MaterialUniformManager>(); }

        MaterialUniformManager();
        ~MaterialUniformManager();

        void init(const LZ::Wrapper::Device::Ptr& device, size_t materialCount, const LZ::Loader::Material* initialData, int frameCount);

        std::vector<LZ::Wrapper::UniformParameter::Ptr> getParams() const;

    private:

        LZ::Wrapper::Device::Ptr mDevice{ nullptr };
        LZ::Wrapper::UniformParameter::Ptr mMaterialParam{ nullptr };
    };

}