#pragma once

#include "../base.h"
#include "../vulkanWrapper/device.h"
#include "../vulkanWrapper/buffer.h"
#include "../vulkanWrapper/description.h"
#include "../vulkanWrapper/descriptorSet.h"
#include "../scene/mesh.h"

namespace FF {


    class MaterialUniformManager {
    public:
        using Ptr = std::shared_ptr<MaterialUniformManager>;
        static Ptr create() { return std::make_shared<MaterialUniformManager>(); }

        MaterialUniformManager();
        ~MaterialUniformManager();

        void init(const Wrapper::Device::Ptr& device, size_t materialCount, const Material* initialData, int frameCount);

        std::vector<Wrapper::UniformParameter::Ptr> getParams() const;

    private:

        Wrapper::Device::Ptr mDevice{ nullptr };
        Wrapper::UniformParameter::Ptr mMaterialParam{ nullptr };
    };

}