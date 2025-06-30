#include "transformUniformManager.h"

namespace FF {

    TransformUniformManager::TransformUniformManager(){}
    TransformUniformManager::~TransformUniformManager(){}

    void TransformUniformManager::init(const Wrapper::Device::Ptr& device, size_t transformCount, const glm::mat4* initialData, int frameCount) {
        
        mDevice = device;

        mTransformParam = Wrapper::UniformParameter::create();
        mTransformParam->mBinding = 1;
        mTransformParam->mDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        mTransformParam->mStage = VK_SHADER_STAGE_VERTEX_BIT;
        mTransformParam->mCount = 1;
        mTransformParam->mSize = sizeof(glm::mat4) * transformCount;

        for (int i = 0; i < frameCount; ++i) {
            auto buffer = Wrapper::Buffer::createStorageBuffer(
                device,
                mTransformParam->mSize,
                initialData,
                false
            );
            mTransformParam->mBuffers.push_back(buffer);
        }
    }

    std::vector<Wrapper::UniformParameter::Ptr> TransformUniformManager::getParams() const {
        return { mTransformParam };
    }

}

