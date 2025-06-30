#include "drawDataUniformManager.h"

namespace LZ::Renderer {

    DrawDataUniformManager::DrawDataUniformManager() {}
    DrawDataUniformManager::~DrawDataUniformManager() {}

    void DrawDataUniformManager::init(const LZ::Wrapper::Device::Ptr& device,
        size_t drawCount,
        const LZ::Loader::DrawData* initialData,
        int frameCount) {

        mDevice = device;

        mDrawDataParam = Wrapper::UniformParameter::create();
        mDrawDataParam->mBinding = 4;
        mDrawDataParam->mDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        mDrawDataParam->mStage = VK_SHADER_STAGE_VERTEX_BIT;
        mDrawDataParam->mCount = 1;
        mDrawDataParam->mSize = sizeof(LZ::Loader::DrawData) * drawCount;

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
            const LZ::Loader::DrawData* drawData = reinterpret_cast<const LZ::Loader::DrawData*>(initialData);
            size_t count = mDrawDataParam->mSize / sizeof(LZ::Loader::DrawData);
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

