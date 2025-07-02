#pragma once

#include "../../common.h"
#include "../../wrapper/device.h"
#include "../../wrapper/buffer.h"
#include "../../wrapper/description.h"
#include "../../wrapper//descriptor_set.h"
#include "../../loader/mesh.h"

namespace lzvk::renderer {


    class MaterialUniformManager {
    public:
        using Ptr = std::shared_ptr<MaterialUniformManager>;
        static Ptr create() { return std::make_shared<MaterialUniformManager>(); }

        MaterialUniformManager();
        ~MaterialUniformManager();

        void init(const lzvk::wrapper::Device::Ptr& device, size_t materialCount, const lzvk::loader::Material* initialData, int frameCount);

        std::vector<lzvk::wrapper::UniformParameter::Ptr> getParams() const;

    private:

        lzvk::wrapper::Device::Ptr mDevice{ nullptr };
        lzvk::wrapper::UniformParameter::Ptr mMaterialParam{ nullptr };
    };

}