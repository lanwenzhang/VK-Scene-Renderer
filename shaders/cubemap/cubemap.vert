#version 460

layout(binding = 0) uniform VPMatrices {
    mat4 mViewMatrix;
    mat4 mProjectionMatrix;
} vpUBO;


layout(location = 0) out vec3 vDirection;

const vec3 positions[8] = vec3[](
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0,  1.0),
    vec3( 1.0,  1.0,  1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3(-1.0, -1.0, -1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3(-1.0,  1.0, -1.0)
);

const int indices[36] = int[](
    0, 1, 2, 2, 3, 0,    // front
    1, 5, 6, 6, 2, 1,    // right
    7, 6, 5, 5, 4, 7,    // back
    4, 0, 3, 3, 7, 4,    // left
    4, 5, 1, 1, 0, 4,    // bottom
    3, 2, 6, 6, 7, 3     // top
);


mat3 rotateX90() {
    return mat3(
        1.0,  0.0,  0.0,
        0.0,  0.0,  1.0,
        0.0, -1.0,  0.0
    );
}

void main() {

    int idx = indices[gl_VertexIndex];
    vec3 pos = positions[idx];
    vDirection = rotateX90() * pos;
    mat4 viewRotOnly = mat4(mat3(vpUBO.mViewMatrix));
    gl_Position = vpUBO.mProjectionMatrix * viewRotOnly * vec4(pos, 1.0);
}
