#version 450

layout (set = 0, binding = 0) uniform sampler2D texColor;
layout (set = 0, binding = 1) uniform sampler2D texSSAO;

layout (push_constant) uniform CombineParams {
    float scale;
    float bias;
} pc;

layout (location = 0) in vec2 vUV;
layout (location = 0) out vec4 fragColor;

void main() {

    vec3 color = texture(texColor, vUV).rgb;
    float ssao = clamp(texture(texSSAO, vUV).r + pc.bias, 0.0, 1.0);
    
    fragColor = vec4(mix(color, color * ssao, pc.scale).rgb, 1.0);
    
}
   
