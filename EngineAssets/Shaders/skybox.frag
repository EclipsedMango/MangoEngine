#version 460 core

in vec3 v_TexCoords;
out vec4 FragColor;

uniform samplerCube u_Skybox;

void main() {
    vec3 hdr = texture(u_Skybox, normalize(v_TexCoords)).rgb;
    FragColor = vec4(hdr * 0.5, 1.0);
}
