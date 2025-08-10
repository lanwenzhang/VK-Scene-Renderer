#version 450

layout(location = 6) in flat uint matID;

struct Material {

    vec4 emissiveFactor;
    vec4 baseColorFactor;

    float roughness;
    float metallicFactor;
    float alphaTest;
    float transparencyFactor;

    uint baseColorTexture;
    uint specularTexture;
    uint emissiveTexture;
    uint normalTexture;
    uint opacityTexture;
    uint occlusionTexture;
};

layout(set = 1, binding = 2) readonly buffer MaterialParams { Material materials[]; };

void main(){
    
    if(materials[matID].alphaTest > 0.5) discard;
}