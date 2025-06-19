#version 460

layout(location = 0) in vec3 vDirection;
layout(location = 0) out vec4 outColor;

layout(binding = 3) uniform samplerCube uCubemap;

void main() {

    outColor = texture(uCubemap, normalize(vDirection));
}