#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace LZ::Core {

	struct VPMatrices {
		glm::mat4 mViewMatrix;
		glm::mat4 mProjectionMatrix;

		VPMatrices() {
			mViewMatrix = glm::mat4(1.0f);
			mProjectionMatrix = glm::mat4(1.0f);
		}
	};

	struct ObjectUniform {
		glm::mat4 mModelMatrix;

		ObjectUniform() {
			mModelMatrix = glm::mat4(1.0f);
		}
	};

	struct PushConstants {

		glm::mat4 vpMatrices;
		uint64_t bufferPosAngle;
		uint64_t bufferMatrices;
		float time;
		uint32_t instancingCount;
	};

	enum class CAMERA_MOVE
	{
		MOVE_LEFT,
		MOVE_RIGHT,
		MOVE_FRONT,
		MOVE_BACK
	};

}


