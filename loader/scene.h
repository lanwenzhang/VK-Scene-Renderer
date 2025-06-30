#pragma once

#include "../common.h"
#include <assimp/matrix4x4.h>

namespace LZ::Loader {

	struct DrawData {
		uint32_t transformId;
		uint32_t materialId;
	};

	struct Hierarchy {
		int parent = -1;
		int firstChild = -1;
		int nextSibling = -1;
		int lastSibling = -1;
		int level = 0;
	};

	struct Scene {

		std::vector<glm::mat4> localTransform;
		std::vector<glm::mat4> globalTransform;

		std::vector<Hierarchy> hierarchy;

		std::unordered_map<uint32_t, uint32_t> meshForNode;
		std::unordered_map<uint32_t, uint32_t> materialForNode;

		std::unordered_map<uint32_t, uint32_t> nameForNode;
		std::vector<std::string> nodeNames;
		std::vector<std::string> materialNames;
		std::vector<DrawData> drawDataArray;
	};

	int addNode(Scene& scene, int parent, int level);
	glm::mat4 toMat4(const aiMatrix4x4& a);
	bool recalculateGlobalTransforms(Scene& scene);
	void markAsChanged(Scene& scene, int node);
	std::string getNodeName(const Scene& scene, int node);
}




