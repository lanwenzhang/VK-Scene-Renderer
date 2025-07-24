#pragma once

#include <glm/glm.hpp>
#include <cstdint>

namespace lzvk::renderer::gpu
{
    struct GpuMaterial {
    
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
        uint32_t padding[2];
    };

} 
