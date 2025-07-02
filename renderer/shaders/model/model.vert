#version 450

#extension GL_ARB_separate_shader_objects:enable

layout(location = 0) in vec3 inPosition;
layout(location = 2) in vec2 inUV;

layout(location = 1) out vec2 outUV;


layout(binding = 0) uniform VPMatrices{
	
	mat4 mViewMatrix;
	mat4 mProjectionMatrix;

}vpUBO;


layout(std430, binding = 1) readonly buffer Matrices {
    mat4 model[];
};


void main() {
    mat4 modelMatrix = model[gl_InstanceIndex];
    gl_Position = vpUBO.mProjectionMatrix * vpUBO.mViewMatrix * modelMatrix * vec4(inPosition, 1.0);
    outUV = inUV;
}