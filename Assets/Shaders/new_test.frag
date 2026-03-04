#version 460 core
//#define USE_RED
#include "hello.comp"
#define HELLO 1.0

in vec3 v_Normal;
in vec2 v_TexCoord;
in vec3 v_FragPos;

out vec4 FragColor;

void main() {
    float alpha = HELLO;
    FragColor = vec4(hello(), alpha);
}