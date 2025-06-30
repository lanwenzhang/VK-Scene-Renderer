#pragma once

#include "base.h"

class PushConstantsManager {
public:
	
	using Ptr = std::shared_ptr<PushConstantsManager>;
	static Ptr create() { return std::make_shared<PushConstantsManager>(); }

	PushConstantsManager();
	~PushConstantsManager();

	void update(const glm::mat4& viewproj, uint64_t posAngleAddr, uint64_t matrixAddr, float timeSec, uint32_t instancingCount);
	void update(const glm::mat4& mvp);


	const PushConstants& get() const { return mPushConstants; }

private:
	
	PushConstants mPushConstants;
};