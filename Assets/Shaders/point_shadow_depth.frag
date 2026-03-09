#version 460 core
in vec3 v_WorldPos;

uniform vec3  u_LightPos;
uniform float u_FarPlane;

void main() {
    float dist = length(v_WorldPos - u_LightPos);
    // store linear radial depth in [0,1]
    gl_FragDepth = dist / u_FarPlane;
}