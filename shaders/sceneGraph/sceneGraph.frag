#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragUV;
layout(location = 3) in flat uint matID;

layout(location = 0) out vec4 outColor;

struct Material {
    vec4 emissiveFactor;
    vec4 baseColorFactor;

    float roughness;
    float metallicFactor;
    float alphaTest;
    float transparencyFactor;

    uint baseColorTexture;
    uint emissiveTexture;
    uint normalTexture;
    uint opacityTexture;
    uint occlusionTexture;
};

layout(set = 1, binding = 2) readonly buffer MaterialParams { Material materials[]; };

layout(set = 2, binding = 0) uniform sampler2D diffuseMaps[];


void main()
{

    vec3 texColor = texture(diffuseMaps[nonuniformEXT(matID)], fragUV).rgb;
    outColor = vec4(texColor, 1.0);
}


