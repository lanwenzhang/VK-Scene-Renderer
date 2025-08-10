#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 6) out flat uint matID;

struct DrawData {
    uint transformId;
    uint materialId;
};


layout(set = 0, binding = 1) uniform VPMatrices {
    mat4 mViewMatrix;
    mat4 mProjectionMatrix;
} lightvp;

layout(set = 1, binding = 1) readonly buffer Transforms { mat4 worldMatrices[];};
layout(set = 1, binding = 4) readonly buffer DrawDataBuffer { DrawData dd[]; };


void main() {

    uint transformIndex = dd[gl_BaseInstance].transformId;
    mat4 model = worldMatrices[transformIndex];
    vec4 worldPos = model * vec4(inPosition, 1.0);
    
    matID = dd[gl_BaseInstance].materialId;

    gl_Position = lightvp.mProjectionMatrix * lightvp.mViewMatrix * worldPos;

}

