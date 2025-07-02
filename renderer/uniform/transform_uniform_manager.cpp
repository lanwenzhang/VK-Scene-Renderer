#include "transform_uniform_manager.h"

namespace lzvk::renderer {

    TransformUniformManager::TransformUniformManager(){}
    TransformUniformManager::~TransformUniformManager(){}

    void TransformUniformManager::init(const lzvk::wrapper::Device::Ptr& device, size_t transformCount, const glm::mat4* initialData, int frameCount) {
        
        mDevice = device;

        mTransformParam = lzvk::wrapper::UniformParameter::create();
        mTransformParam->mBinding = 1;
        mTransformParam->mDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        mTransformParam->mStage = VK_SHADER_STAGE_VERTEX_BIT;
        mTransformParam->mCount = 1;
        mTransformParam->mSize = sizeof(glm::mat4) * transformCount;

        for (int i = 0; i < frameCount; ++i) {
            auto buffer = lzvk::wrapper::Buffer::createStorageBuffer(
                device,
                mTransformParam->mSize,
                initialData,
                false
            );
            mTransformParam->mBuffers.push_back(buffer);
        }
    }

    std::vector<lzvk::wrapper::UniformParameter::Ptr> TransformUniformManager::getParams() const {
        return { mTransformParam };
    }

}

