#pragma once

#include "../common.h"
#include "scene.h"

namespace lzvk::loader {

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

        uint32_t baseColorTexture = static_cast<uint32_t>(-1);
        uint32_t specularTexture = static_cast<uint32_t>(-1);
        uint32_t emissiveTexture = static_cast<uint32_t>(-1);
        uint32_t normalTexture = static_cast<uint32_t>(-1);
        uint32_t opacityTexture = static_cast<uint32_t>(-1);
        uint32_t occlusionTexture = static_cast<uint32_t>(-1);

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
        std::vector<std::string> normalTextureFiles;
        std::vector<std::string> occlusionTextureFiles;
    };

    bool loadMeshFile(const std::string& path, MeshData& meshData, Scene& scene);
    bool loadMeshData(const std::string& path, MeshData& meshData);
    void saveMeshData(const std::string& path, const MeshData& meshData);
}

