#version 460 core

in vec2 v_TexCoord;

uniform bool u_AlphaScissor;
uniform float u_AlphaScissorThreshold;
uniform bool u_HasDiffuse;
uniform sampler2D u_Diffuse;
uniform vec4 u_AlbedoColor;
uniform vec2 u_UVScale;
uniform vec2 u_UVOffset;

void main() {
    if (u_AlphaScissor && u_HasDiffuse) {
        vec2 uv = v_TexCoord * u_UVScale + u_UVOffset;
        float alpha = texture(u_Diffuse, uv).a * u_AlbedoColor.a;
        if (alpha < u_AlphaScissorThreshold) {
            discard;
        }
    }
}