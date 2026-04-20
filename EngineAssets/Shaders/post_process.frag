#version 460 core

in vec2 v_Uv;
out vec4 FragColor;

uniform sampler2D u_SceneColor;
uniform float u_Exposure;
uniform int u_TonemapMode; // 0: Linear, 1: Reinhard, 2: Filmic, 3: ACES, 4: Stylized

vec3 RRTAndODTFit(vec3 v) {
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

vec3 ACESFitted(vec3 color) {
    const mat3 ACESInputMat = mat3(
        vec3(0.59719, 0.07600, 0.02840),
        vec3(0.35458, 0.90834, 0.13383),
        vec3(0.04823, 0.01566, 0.83777)
    );

    const mat3 ACESOutputMat = mat3(
        vec3(1.60475, -0.10208, -0.00327),
        vec3(-0.53108, 1.10813, -0.07276),
        vec3(-0.07367, -0.00605, 1.07602)
    );

    color = ACESInputMat * color;
    color = RRTAndODTFit(color);
    color = ACESOutputMat * color;
    return clamp(color, 0.0, 1.0);
}

vec3 ReinhardTonemap(vec3 color) {
    return color / (vec3(1.0) + color);
}

vec3 FilmicTonemap(vec3 color) {
    // uncharted 2/Hable filmic curve, normalized so white stays near white.
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;
    const float W = 11.2;

    vec3 x = max(color, vec3(0.0));
    vec3 mapped = ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
    float whiteScale = 1.0 / ((((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F));
    return clamp(mapped * whiteScale, 0.0, 1.0);
}

vec3 StylizedTonemap(vec3 color) {
    vec3 mapped = ACESFitted(color);
    float luma = dot(mapped, vec3(0.2126, 0.7152, 0.0722));
    mapped = mix(vec3(luma), mapped, 1.6);
    mapped = mapped / (mapped + 0.15) * 1.15;
    mapped = smoothstep(vec3(0.02), vec3(1.0), mapped);
    return clamp(mapped, 0.0, 1.0);
}

vec3 ApplyTonemap(vec3 color) {
    if (u_TonemapMode == 0) {
        return clamp(color, 0.0, 1.0);
    }
    if (u_TonemapMode == 1) {
        return ReinhardTonemap(color);
    }
    if (u_TonemapMode == 2) {
        return FilmicTonemap(color);
    }
    if (u_TonemapMode == 4) {
        return StylizedTonemap(color);
    }
    return ACESFitted(color);
}

void main() {
    vec4 hdr = texture(u_SceneColor, v_Uv);
    vec3 mapped = ApplyTonemap(hdr.rgb * u_Exposure);
    vec3 displayColor = pow(mapped, vec3(1.0 / 2.2));
    FragColor = vec4(displayColor, hdr.a);
}
