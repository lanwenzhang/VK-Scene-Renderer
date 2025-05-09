#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <array>
#include <map>
#include <string>
#include <optional>
#include <set>
#include <fstream>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

// Validation layer
const std::vector<const char*> validationLayers = {

	"VK_LAYER_KHRONOS_validation"
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