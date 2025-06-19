# 1 Set verison and project name
cmake_minimum_required(VERSION 3.12)
project(Vulkan)
set(CMAKE_CXX_STANDARD 17)

# 2 Copy assets
file(GLOB ASSETS "./assets")
file(COPY ${ASSETS} DESTINATION ${CMAKE_BINARY_DIR})

# 3 Third party
# 3.1 Include third party .h
include_directories(
SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/assimp/include
SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/glm/include
SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/glfw/include
SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/vulkan/Include
)

# 3.2 Link third party .lib
link_directories(
SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/assimp/lib
SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/glfw/lib
SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/vulkan/Lib
)

# 4 Add subdirectory .cpp
add_subdirectory(vulkanWrapper)
add_subdirectory(texture)
add_subdirectory(tools)

# 5 Main directory
# 5.1 Collect all .cpp and .c  to variable
aux_source_directory(. DIRSRCS)

# 5.2 Compile variable to .exe
add_executable(Vulkan ${DIRSRCS})

# 6 Link subdirectory .cpp and third party .lib
target_link_libraries(Vulkan vulkanLib textureLib toolsLib vulkan-1.lib glfw3.lib assimp-vc143-mtd.lib zlibstaticd.lib)