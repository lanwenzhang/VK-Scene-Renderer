#include "frameUniformManager.h"

namespace FF {

    FrameUniformManager::FrameUniformManager(){}
    FrameUniformManager::~FrameUniformManager(){}

    void FrameUniformManager::init(const FF::Wrapper::Device::Ptr& device, int frameCount) {
       
        mDevice = device;

        mVpParam = FF::Wrapper::UniformParameter::create();
        mVpParam->mBinding = 0;
        mVpParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mVpParam->mStage = VK_SHADER_STAGE_VERTEX_BIT;
        mVpParam->mCount = 1;
        mVpParam->mSize = sizeof(VPMatrices);

        for (int i = 0; i < frameCount; ++i) {

            auto buffer = FF::Wrapper::Buffer::createUniformBuffer(device, mVpParam->mSize, nullptr);
            mVpParam->mBuffers.push_back(buffer);
        }
    }

    void FrameUniformManager::update(const glm::mat4& view, const glm::mat4& projection, const Wrapper::DescriptorSet::Ptr& descriptorSet, int frameIndex) {
        
        VPMatrices vp{};
        vp.mViewMatrix = view;
        vp.mProjectionMatrix = projection;

        mVpParam->mBuffers[frameIndex]->updateBufferByMap(&vp, sizeof(VPMatrices));

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mVpParam->mBuffers[frameIndex]->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(VPMatrices);

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

    std::vector<FF::Wrapper::UniformParameter::Ptr> FrameUniformManager::getParams() const {
       
        return { mVpParam };
    }

}
