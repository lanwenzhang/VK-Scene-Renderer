#include "material_uniform_manager.h"
#include "../gpu_structs.h"

namespace lzvk::renderer {

    MaterialUniformManager::MaterialUniformManager(){}
    MaterialUniformManager::~MaterialUniformManager(){}

    void MaterialUniformManager::init(
        const lzvk::wrapper::Device::Ptr& device,
        size_t materialCount,
        const lzvk::loader::Material* initialData,
        int frameCount)
    {
        mDevice = device;

        using namespace lzvk::renderer::gpu;

        std::vector<GpuMaterial> gpuMaterials;
        gpuMaterials.reserve(materialCount);

        if (initialData)
        {
            for (size_t i = 0; i < materialCount; ++i)
            {
                const auto& cpuMat = initialData[i];

                GpuMaterial gpuMat;
                gpuMat.emissiveFactor = cpuMat.emissiveFactor;
                gpuMat.baseColorFactor = cpuMat.baseColorFactor;
                gpuMat.roughness = cpuMat.roughness;
                gpuMat.metallicFactor = cpuMat.metallicFactor;
                gpuMat.alphaTest = cpuMat.alphaTest;
                gpuMat.transparencyFactor = cpuMat.transparencyFactor;
                gpuMat.baseColorTexture = cpuMat.baseColorTexture;
                gpuMat.specularTexture = cpuMat.specularTexture;
                gpuMat.emissiveTexture = cpuMat.emissiveTexture;
                gpuMat.normalTexture = cpuMat.normalTexture;
                gpuMat.opacityTexture = cpuMat.opacityTexture;
                gpuMat.occlusionTexture = cpuMat.occlusionTexture;

                gpuMaterials.push_back(gpuMat);
            }
        }

        mMaterialParam = lzvk::wrapper::UniformParameter::create();
        mMaterialParam->mBinding = 2;
        mMaterialParam->mDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        mMaterialParam->mStage = VK_SHADER_STAGE_FRAGMENT_BIT;
        mMaterialParam->mCount = 1;
        mMaterialParam->mSize = sizeof(GpuMaterial) * materialCount;

        for (int i = 0; i < frameCount; ++i)
        {
            auto buffer = lzvk::wrapper::Buffer::createStorageBuffer(
                device,
                mMaterialParam->mSize,
                gpuMaterials.data(),
                false
            );
            mMaterialParam->mBuffers.push_back(buffer);
        }
    }
        

    std::vector<lzvk::wrapper::UniformParameter::Ptr> MaterialUniformManager::getParams() const {
        
        return { mMaterialParam };

    }
}
