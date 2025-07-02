#include "material_uniform_manager.h"


namespace lzvk::renderer {

    MaterialUniformManager::MaterialUniformManager(){}
    MaterialUniformManager::~MaterialUniformManager(){}

    void MaterialUniformManager::init(const lzvk::wrapper::Device::Ptr& device, size_t materialCount, const lzvk::loader::Material* initialData, int frameCount) {
        
        mDevice = device;

        mMaterialParam = lzvk::wrapper::UniformParameter::create();
        mMaterialParam->mBinding = 2;
        mMaterialParam->mDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        mMaterialParam->mStage = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        mMaterialParam->mCount = 1;
        mMaterialParam->mSize = sizeof(lzvk::loader::Material) * materialCount;

        for (int i = 0; i < frameCount; ++i) {
            auto buffer = lzvk::wrapper::Buffer::createStorageBuffer(
                device,
                mMaterialParam->mSize,
                initialData,
                false
            );
            mMaterialParam->mBuffers.push_back(buffer);
        }
    }


    std::vector<lzvk::wrapper::UniformParameter::Ptr> MaterialUniformManager::getParams() const {
        return { mMaterialParam };
    }
}
