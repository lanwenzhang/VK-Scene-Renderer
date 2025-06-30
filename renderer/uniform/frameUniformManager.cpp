#include "frameUniformManager.h"

namespace LZ::Renderer {

    FrameUniformManager::FrameUniformManager(){}
    FrameUniformManager::~FrameUniformManager(){}

    void FrameUniformManager::init(const LZ::Wrapper::Device::Ptr& device, int frameCount) {
       
        mDevice = device;

        mVpParam = LZ::Wrapper::UniformParameter::create();
        mVpParam->mBinding = 0;
        mVpParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mVpParam->mStage = VK_SHADER_STAGE_VERTEX_BIT;
        mVpParam->mCount = 1;
        mVpParam->mSize = sizeof(LZ::Core::VPMatrices);

        for (int i = 0; i < frameCount; ++i) {

            auto buffer = LZ::Wrapper::Buffer::createUniformBuffer(device, mVpParam->mSize, nullptr);
            mVpParam->mBuffers.push_back(buffer);
        }
    }

    void FrameUniformManager::update(const glm::mat4& view, const glm::mat4& projection, const LZ::Wrapper::DescriptorSet::Ptr& descriptorSet, int frameIndex) {
        
        LZ::Core::VPMatrices vp{};
        vp.mViewMatrix = view;
        vp.mProjectionMatrix = projection;

        mVpParam->mBuffers[frameIndex]->updateBufferByMap(&vp, sizeof(LZ::Core::VPMatrices));

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mVpParam->mBuffers[frameIndex]->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(LZ::Core::VPMatrices);

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = descriptorSet->getDescriptorSet(frameIndex);
        write.dstBinding = mVpParam->mBinding;
        write.dstArrayElement = 0;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.descriptorCount = 1;
        write.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(
            mDevice->getDevice(),
            1,
            &write,
            0,
            nullptr);
    }

    std::vector<LZ::Wrapper::UniformParameter::Ptr> FrameUniformManager::getParams() const {
       
        return { mVpParam };
    }

}
