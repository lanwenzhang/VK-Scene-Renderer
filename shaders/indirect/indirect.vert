#version 450
#extension GL_ARB_gpu_shader_int64 : enable

layout(push_constant) uniform PushConstant {
    mat4 mvp;
    uint64_t bufferPosAngle;
    uint64_t bufferMatrices;
    float time;
    uint instancingCount;
} pc;
layout(location = 0) in vec3 inPosition;

void main(){

	gl_Position = pc.mvp * vec4(inPosition, 1.0);
}