#include "pushConstantsManager.h"

PushConstantsManager::PushConstantsManager(){}
PushConstantsManager::~PushConstantsManager(){}

void PushConstantsManager::update(const glm::mat4& viewproj, uint64_t posAngleAddr, uint64_t matrixAddr,float timeSec, uint32_t instancingCount) {
    
    mPushConstants.vpMatrices = viewproj;
    mPushConstants.bufferPosAngle = posAngleAddr;
    mPushConstants.bufferMatrices = matrixAddr;
    mPushConstants.time = timeSec;
    mPushConstants.instancingCount = instancingCount;
}

void PushConstantsManager::update(const glm::mat4& mvp) {
    
    mPushConstants.vpMatrices = mvp;
}