#include "materialUniformManager.h"


namespace FF {

    MaterialUniformManager::MaterialUniformManager(){}
    MaterialUniformManager::~MaterialUniformManager(){}

    void MaterialUniformManager::init(const Wrapper::Device::Ptr& device, size_t materialCount, const Material* initialData, int frameCount) {
        
        mDevice = device;

        mMaterialParam = Wrapper::UniformParameter::create();
        mMaterialParam->mBinding = 2;
        mMaterialParam->mDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        mMaterialParam->mStage = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        mMaterialParam->mCount = 1;
        mMaterialParam->mSize = sizeof(Material) * materialCount;

        for (int i = 0; i < frameCount; ++i) {
            auto buffer = Wrapper::Buffer::createStorageBuffer(
                device,
                mMaterialParam->mSize,
                initialData,
                false
            );
            mMaterialParam->mBuffers.push_back(buffer);
        }
    }


    std::vector<Wrapper::UniformParameter::Ptr> MaterialUniformManager::getParams() const {
        return { mMaterialParam };
    }
}
