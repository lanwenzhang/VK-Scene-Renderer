#include "instancing_manager.h"

namespace lzvk::renderer {

    InstancingManager::InstancingManager() {}
    InstancingManager::~InstancingManager() {}

    bool InstancingManager::init(const lzvk::wrapper::Device::Ptr& device, uint32_t instancingCount, uint32_t frameCount) {

        mDevice = device;
        mInstancingCount = instancingCount;
        mFrameCount = frameCount;

        // 1. Generate random positions and angles for all instanced meshes
        mPositionsAngles.resize(mInstancingCount);
        for (auto& p : mPositionsAngles) {

            glm::vec3 pos = glm::linearRand(glm::vec3(-50.0f), glm::vec3(50.0f));
            float angle = glm::linearRand(0.0f, glm::pi<float>());
            p = glm::vec4(pos, angle);
        }

        mBufferPosAngle = lzvk::wrapper::Buffer::createStorageBuffer(mDevice, sizeof(glm::vec4) * mInstancingCount, mPositionsAngles.data(), true);


        if (!mBufferPosAngle || mBufferPosAngle->getBuffer() == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to create mBufferPosAngle or buffer handle is null");
        }

        // 2 Each frame one inital model matrix
        mBufferMatrices.resize(mFrameCount);
        for (uint32_t i = 0; i < frameCount; ++i) {

            mBufferMatrices[i] = lzvk::wrapper::Buffer::createStorageBuffer(mDevice, sizeof(glm::mat4) * mInstancingCount, nullptr, true);

            if (!mBufferMatrices[i] || mBufferMatrices[i]->getBuffer() == VK_NULL_HANDLE) {
                throw std::runtime_error("Failed to create mBufferMatrices[" + std::to_string(i) + "]");
            }
        }

        return true;
    }

}

