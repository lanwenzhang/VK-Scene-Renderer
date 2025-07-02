#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec2 fragUV;
layout(location = 3) out flat uint matID;

struct DrawData {
    uint transformId;
    uint materialId;
};


layout(set = 0, binding = 0) uniform VPMatrices {
    mat4 mViewMatrix;
    mat4 mProjectionMatrix;
} vpUBO;

layout(set = 1, binding = 1) readonly buffer Transforms {
    mat4 worldMatrices[];
};

layout(set = 1, binding = 4) readonly buffer DrawDataBuffer {
    DrawData dd[]; 
};

void main() {

    uint transformIndex = dd[gl_BaseInstance].transformId;
    mat4 model = worldMatrices[transformIndex];
    gl_Position = vpUBO.mProjectionMatrix * vpUBO.mViewMatrix * model * vec4(inPosition, 1.0);
    fragUV = inUV;
    matID = dd[gl_BaseInstance].materialId;
}
