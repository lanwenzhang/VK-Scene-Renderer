#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace lzvk::core {

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

	struct SSAOPushConstants {
		float zNear;
		float zFar;
		float radius;
		float attScale;
		float distScale;
	};

	struct BlurPushConstant {
		float depthThreshold;
	};

	struct CombinePushConstant {
		float scale = 1.5f;
		float bias = 0.16f;
	};


	enum class PipelineType {
		SceneGraph,
		Skybox,
		Combine,
		Blur
	};

	enum class CAMERA_MOVE
	{
		MOVE_LEFT,
		MOVE_RIGHT,
		MOVE_FRONT,
		MOVE_BACK
	};

}


