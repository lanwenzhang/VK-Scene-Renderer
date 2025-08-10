#version 450

#extension GL_EXT_nonuniform_qualifier : enable

precision highp float;
precision mediump int;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in mat3 tbn;
layout(location = 6) in flat uint matID;
layout(location = 7) in vec4 lightSpaceClipCoord;
layout(location = 8) in vec4 worldPos;

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
layout(set = 6, binding = 0) uniform sampler2D specularTextures[];
layout(set = 7, binding = 0) uniform sampler2D shadowMap;
layout(set = 8, binding = 4) uniform samplerCube irradianceMap;


layout(push_constant) uniform PushConstants {
    
    vec4 lightDir;
    vec4 cameraPos;

} pc;


float pcf(vec3 uvw){

    float size = 1.0 / textureSize(shadowMap, 0).x;
    float shadow = 0.0;

    for (int i=-1; i<=+1; i++){
      for (int j=-1; j<=+1; j++){

            vec2 offset = size * vec2(i, j);
            float sampledDepth = texture(shadowMap, uvw.xy + offset).r;

            shadow += uvw.z > sampledDepth ? 0.0 : 1.0;
      }
    }
    return shadow / 9;
}

float shadow(vec4 s){

   s = s / s.w;
   vec3 uvw = vec3(s.xy * 0.5 + 0.5, s.z);
   
   if (uvw.x >= 0.0 && uvw.x <= 1.0 &&
       uvw.y >= 0.0 && uvw.y <= 1.0 &&
       uvw.z >= 0.0 && uvw.z <= 1.0)
      {
        float shadowSample = pcf(uvw);
        return mix(0.3, 1.0, shadowSample);
      }

  return 1.0;

}

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

vec3 rotateForIrradiance(vec3 dir) {

    mat3 rot = mat3(
        1.0, 0.0,  0.0,
        0.0, 0.0,  -1.0,
        0.0, 1.0, 0.0
    );
    return normalize(rot * dir);
}


mat3 rotY180 = mat3(
    -1.0, 0.0,  0.0,
     0.0, 1.0,  0.0,
     0.0, 0.0, -1.0
);


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


        vec4 specular = vec4(0.0, 0.0, 0.0, 0.0);
    if (materials[matID].specularTexture > 0) {
    
        specular = texture(specularTextures[materials[matID].specularTexture], fragUV);
        vec3 viewDir = normalize(pc.cameraPos.xyz - worldPos.xyz);
        vec3 lightDir = -normalize(pc.lightDir.xyz);
        vec3 halfwayDir = normalize(lightDir + viewDir);

        float NdotH = clamp(dot(normal, halfwayDir), 0.0, 1.0);
        float specStrength = pow(NdotH, 16.0);
        specular *= specStrength;
    
    }


    vec4 emissive = vec4(0.0, 0.0, 0.0, 0.0);
    if (materials[matID].emissiveTexture > 0) {

        emissive = texture(emissiveTextures[materials[matID].emissiveTexture], fragUV);
    }


    // direct light
    // diffuse
    const vec4 f0 = vec4(0.04);
    float NdotL = clamp(dot(normal, -normalize(pc.lightDir.xyz)), 0.1, 1.0);
    vec4 direct = baseColor * NdotL * (1 - f0);

    // specular
    direct += specular;

    // ambient light
    // diffuse
    normal = rotY180 * normal;
    vec4 irradiance = texture(irradianceMap, normal);
    vec4 ambient = baseColor  * irradiance * (1 - f0);

    vec4 diffuse = direct + ambient;

    outColor = diffuse * shadow(lightSpaceClipCoord) + emissive;

}




