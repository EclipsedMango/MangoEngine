#version 460 core

invariant gl_Position;

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;

layout(std140, binding = 0) uniform CameraData {
    mat4 view;
    mat4 proj;
};

uniform mat4 u_Model;
uniform bool u_UseSkinnedVertexBuffer;
uniform bool u_UseInstancing;

struct SkinnedVertex {
    vec4 position;
    vec4 normal;
    vec4 tangent;
};

layout (std430, binding = 11) readonly buffer SkinnedVertexBuffer {
    SkinnedVertex u_SkinnedVertices[];
};

struct InstanceData {
    mat4 model;
    mat4 normalMatrix;
};

layout (std430, binding = 16) readonly buffer InstanceDataBuffer {
    InstanceData u_InstanceData[];
};

out vec2 v_TexCoord;

void main() {
    mat4 modelMatrix = u_Model;
    if (u_UseInstancing) {
        modelMatrix = u_InstanceData[gl_InstanceID].model;
    }

    v_TexCoord = aTexCoord;
    vec4 localPosition = vec4(aPos, 1.0);
    if (u_UseSkinnedVertexBuffer) {
        localPosition = vec4(u_SkinnedVertices[gl_VertexID].position.xyz, 1.0);
    }

    vec4 worldPosition = modelMatrix * localPosition;
    gl_Position = proj * view * worldPosition;
}
