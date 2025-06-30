#pragma once

#include "../base.h"
#include "scene.h"

struct Mesh {
	uint32_t indexOffset = 0;
	uint32_t vertexOffset = 0;
	uint32_t indexCount = 0;
	uint32_t materialID = 0;
};

struct Material {

    glm::vec4 emissiveFactor = glm::vec4(0.0f);
    glm::vec4 baseColorFactor = glm::vec4(1.0f);

    float roughness = 1.0f;
    float metallicFactor = 0.0f;
    float alphaTest = 0.0f;
    float transparencyFactor = 1.0f;

    uint32_t baseColorTexture = -1;
    uint32_t specularTexture = -1;
    uint32_t emissiveTexture = -1;
    uint32_t normalTexture = -1;
    uint32_t opacityTexture = -1;
    uint32_t occlusionTexture = -1;

    std::string baseColorTexturePath;
    std::string specularTexturePath;
    std::string emissiveTexturePath;
    std::string normalTexturePath;
    std::string opacityTexturePath;
    std::string occlusionTexturePath;
};
struct MeshData {
    std::vector<uint32_t> indexData;
    std::vector<uint8_t> vertexData;
    std::vector<Mesh> meshes;
    std::vector<Material> materials;

    std::vector<std::string> diffuseTextureFiles;
    std::vector<std::string> emissiveTextureFiles;
    std::vector<std::string> occlusionTextureFiles;
};

bool loadMeshFile(const std::string& path, MeshData& meshData, Scene& scene);