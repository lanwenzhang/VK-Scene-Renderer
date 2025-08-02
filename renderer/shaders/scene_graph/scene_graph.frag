#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in mat3 tbn;
layout(location = 6) in flat uint matID;


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
layout(set = 5, binding = 0) uniform sampler2D opacityTextures[];


void runAlphaTest(float alpha, float alphaThreshold)
{
  if (alphaThreshold > 0.0) {

    mat4 thresholdMatrix = mat4(
      1.0  / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0,
      13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0,
      4.0  / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0,
      16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0
    );

    alpha = clamp(alpha - 0.5 * thresholdMatrix[int(mod(gl_FragCoord.x, 4.0))][int(mod(gl_FragCoord.y, 4.0))], 0.0, 1.0);

    if (alpha < alphaThreshold)
      discard;
  }
}


void main()
{

    vec4 baseColor = materials[matID].baseColorFactor;
    if(materials[matID].baseColorTexture > 0){
    
        baseColor *= texture(diffuseTextures[materials[matID].baseColorTexture], fragUV);
    }

    if (materials[matID].opacityTexture > 0) {
    baseColor.a = texture(opacityTextures[materials[matID].opacityTexture], fragUV).r;
    }


    runAlphaTest(baseColor.a, materials[matID].alphaTest / max(32.0 * fwidth(fragUV.x), 1.0));

    vec3 normal = normalize(fragNormal);
    if (materials[matID].normalTexture > 0) {
        vec3 sampledNormal = texture(normalTextures[materials[matID].normalTexture], fragUV).xyz;
        sampledNormal = sampledNormal * 2.0 - 1.0;
        normal = normalize(tbn * sampledNormal);
    }

    vec4 emissive = vec4(0.0, 0.0, 0.0, 0.0);
    if (materials[matID].emissiveTexture > 0) {
        emissive += texture(emissiveTextures[materials[matID].emissiveTexture], fragUV);
    }

    // ========== HARD-CODED DIRECTIONAL LIGHTS ==========
    vec3 lightDir1 = normalize(vec3(-1.0, 1.0, +0.5));
    vec3 lightDir2 = normalize(vec3(+1.0, 1.0, -0.5));

    float NdotL1 = clamp(dot(normal, lightDir1), 0.1, 1.0);
    float NdotL2 = clamp(dot(normal, lightDir2), 0.1, 1.0);

    float NdotL = NdotL1 + NdotL2;

    vec4 diffuse = baseColor * NdotL;

    outColor = diffuse + emissive;

}




