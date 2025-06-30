#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <string>
#include <optional>
#include <functional>
#include <set>
#include <fstream>
#include <algorithm>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#ifndef VK_PIPELINE_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
#define VK_PIPELINE_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT 0x00000002
#endif

// Validation layer
const std::vector<const char*> validationLayers = {

	"VK_LAYER_KHRONOS_validation"
};

// Camera 
enum class CAMERA_MOVE
{
	MOVE_LEFT,
	MOVE_RIGHT,
	MOVE_FRONT,
	MOVE_BACK
};



// Uniform
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

// Push constants
struct PushConstants {

	glm::mat4 vpMatrices;
	uint64_t bufferPosAngle;
	uint64_t bufferMatrices;
	float time;
	uint32_t instancingCount;
};

// Draw indirect
struct DrawIndexedIndirectCommand {
	uint32_t count;
	uint32_t instanceCount;
	uint32_t firstIndex;
	int32_t baseVertex;
	uint32_t baseInstance;
};

// Scene
struct Hierarchy {
	int parent = -1;
	int firstChild = -1;
	int nextSibling = -1;
	int lastSibling = -1;
	int level = 0;
};