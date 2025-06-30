#include "materialUniformManager.h"


namespace LZ::Renderer {

    MaterialUniformManager::MaterialUniformManager(){}
    MaterialUniformManager::~MaterialUniformManager(){}

    void MaterialUniformManager::init(const LZ::Wrapper::Device::Ptr& device, size_t materialCount, const LZ::Loader::Material* initialData, int frameCount) {
        
        mDevice = device;

        mMaterialParam = LZ::Wrapper::UniformParameter::create();
        mMaterialParam->mBinding = 2;
        mMaterialParam->mDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        mMaterialParam->mStage = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        mMaterialParam->mCount = 1;
        mMaterialParam->mSize = sizeof(LZ::Loader::Material) * materialCount;

        for (int i = 0; i < frameCount; ++i) {
            auto buffer = LZ::Wrapper::Buffer::createStorageBuffer(
                device,
                mMaterialParam->mSize,
                initialData,
                false
            );
            mMaterialParam->mBuffers.push_back(buffer);
        }
    }


    std::vector<LZ::Wrapper::UniformParameter::Ptr> MaterialUniformManager::getParams() const {
        return { mMaterialParam };
    }
}
