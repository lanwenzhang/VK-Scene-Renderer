#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <set>
#include <string>
#include <optional>
#include <functional>
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


// Draw indirect
struct DrawIndexedIndirectCommand {
	uint32_t count;
	uint32_t instanceCount;
	uint32_t firstIndex;
	int32_t baseVertex;
	uint32_t baseInstance;
};

