#include "drawDataUniformManager.h"

namespace FF {


    DrawDataUniformManager::DrawDataUniformManager() {}
    DrawDataUniformManager::~DrawDataUniformManager() {}

    void DrawDataUniformManager::init(const Wrapper::Device::Ptr& device,
        size_t drawCount,
        const DrawData* initialData,
        int frameCount) {

        mDevice = device;

        mDrawDataParam = Wrapper::UniformParameter::create();
        mDrawDataParam->mBinding = 4;
        mDrawDataParam->mDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        mDrawDataParam->mStage = VK_SHADER_STAGE_VERTEX_BIT;
        mDrawDataParam->mCount = 1;
        mDrawDataParam->mSize = sizeof(DrawData) * drawCount;

        for (int i = 0; i < frameCount; ++i) {
            auto buffer = Wrapper::Buffer::createStorageBuffer(
                device,
                mDrawDataParam->mSize,
                initialData,
                false
            );
            mDrawDataParam->mBuffers.push_back(buffer);

        }

        if (initialData) {
            std::cout << "[DEBUG] Checking initialData contents:" << std::endl;
            const DrawData* drawData = reinterpret_cast<const DrawData*>(initialData);
            size_t count = mDrawDataParam->mSize / sizeof(DrawData);
            for (size_t j = 0; j < count; ++j) {
                std::cout << "  DrawData[" << j << "] -> transformId="
                    << drawData[j].transformId
                    << ", materialId="
                    << drawData[j].materialId << std::endl;
            }
        }
    }

    std::vector<Wrapper::UniformParameter::Ptr> DrawDataUniformManager::getParams() const {
        return { mDrawDataParam };
    }

}

