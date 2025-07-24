#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in mat3 tbn;
layout(location = 6) in flat uint matID;
layout(location = 7) in flat uint drawID;

layout(location = 0) out vec4 outColor;

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
layout(set = 2, binding = 0) uniform sampler2D diffuseTextures[];
layout(set = 3, binding = 0) uniform sampler2D emissiveTextures[];
layout(set = 4, binding = 0) uniform sampler2D normalTextures[];


//void main() {
//
//     outColor = vec4(0.0, 1.0, 0.0, 1.0);  
//
//}



void runAlphaTest(float alpha, float threshold)
{
    if (alpha < threshold)
        discard;
}


void main()
{

    vec4 baseColor = materials[matID].baseColorFactor;
    if (materials[matID].baseColorTexture > 0) {

        baseColor *= texture(diffuseTextures[materials[matID].baseColorTexture], fragUV);
    }

    runAlphaTest(baseColor.a, materials[matID].alphaTest);


    vec3 normal = normalize(fragNormal);
    if (materials[matID].normalTexture > 0) {
        vec3 sampledNormal = texture(normalTextures[materials[matID].normalTexture], fragUV).xyz;
        sampledNormal = sampledNormal * 2.0 - 1.0;
        normal = normalize(tbn * sampledNormal);
    }

    vec4 emissive = vec4(materials[matID].emissiveFactor.rgb, 0.0);
    if (materials[matID].emissiveTexture > 0) {
        emissive *= texture(emissiveTextures[materials[matID].emissiveTexture], fragUV);
    }

    // ========== HARD-CODED DIRECTIONAL LIGHTS ==========
    vec3 lightDir1 = normalize(vec3(-1.0, 1.0, +0.5));
    vec3 lightDir2 = normalize(vec3(+1.0, 1.0, -0.5));

    float NdotL1 = clamp(dot(normal, lightDir1), 0.1, 1.0);
    float NdotL2 = clamp(dot(normal, lightDir2), 0.1, 1.0);

    float NdotL = NdotL1 + NdotL2;

    vec3 diffuseColor = baseColor.rgb * NdotL;

    outColor = vec4(diffuseColor, baseColor.a) + emissive;
}




