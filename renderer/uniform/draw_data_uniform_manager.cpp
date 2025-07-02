#include "draw_data_uniform_manager.h"

namespace lzvk::renderer {

    DrawDataUniformManager::DrawDataUniformManager() {}
    DrawDataUniformManager::~DrawDataUniformManager() {}

    void DrawDataUniformManager::init(const lzvk::wrapper::Device::Ptr& device,
        size_t drawCount,
        const lzvk::loader::DrawData* initialData,
        int frameCount) {

        mDevice = device;

        mDrawDataParam = lzvk::wrapper::UniformParameter::create();
        mDrawDataParam->mBinding = 4;
        mDrawDataParam->mDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        mDrawDataParam->mStage = VK_SHADER_STAGE_VERTEX_BIT;
        mDrawDataParam->mCount = 1;
        mDrawDataParam->mSize = sizeof(lzvk::loader::DrawData) * drawCount;

        for (int i = 0; i < frameCount; ++i) {
            auto buffer = lzvk::wrapper::Buffer::createStorageBuffer(
                device,
                mDrawDataParam->mSize,
                initialData,
                false
            );
            mDrawDataParam->mBuffers.push_back(buffer);

        }

        if (initialData) {
            std::cout << "[DEBUG] Checking initialData contents:" << std::endl;
            const lzvk::loader::DrawData* drawData = reinterpret_cast<const lzvk::loader::DrawData*>(initialData);
            size_t count = mDrawDataParam->mSize / sizeof(lzvk::loader::DrawData);
            for (size_t j = 0; j < count; ++j) {
                std::cout << "  DrawData[" << j << "] -> transformId="
                    << drawData[j].transformId
                    << ", materialId="
                    << drawData[j].materialId << std::endl;
            }
        }
    }

    std::vector<lzvk::wrapper::UniformParameter::Ptr> DrawDataUniformManager::getParams() const {
        return { mDrawDataParam };
    }
}

