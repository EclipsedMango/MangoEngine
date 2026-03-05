#ifndef LIGHTING_GLSL
#define LIGHTING_GLSL

uniform sampler2D u_ShadowMap[4];
uniform mat4 u_LightSpaceMatrix[4];

struct DirectionalLight {
    vec4 direction;
    vec4 color;
};

layout (std140, binding = 1) uniform GlobalLightData {
    ivec4 u_LightCounts;
    DirectionalLight u_DirLights[4];
};

struct PointLight {
    vec4 position;
    vec4 color;
    vec4 attenuation;
};

layout (std430, binding = 2) readonly buffer PointLightBuffer {
    PointLight pointLights[];
};

struct SpotLight {
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 params;
};

layout (std430, binding = 3) readonly buffer SpotLightBuffer {
    SpotLight spotLights[];
};

layout (std430, binding = 5) readonly buffer LightIndexBuffer {
    uint globalLightIndexList[];
};

struct LightGrid {
    uint offset;
    uint pointCount;
    uint spotCount;
    uint pad;
};

layout (std430, binding = 6) readonly buffer LightGridBuffer {
    LightGrid lightGrid[];
};

vec3 Heatmap(float t) {
    t = clamp(t, 0.0, 1.0);
    vec3 cold = vec3(0.0, 0.0, 1.0);
    vec3 mid  = vec3(0.0, 1.0, 0.0);
    vec3 hot  = vec3(1.0, 0.0, 0.0);
    if (t < 0.5) return mix(cold, mid, t * 2.0);
    return mix(mid, hot, (t - 0.5) * 2.0);
}

vec3 ACESFilmic(vec3 x) {
    return clamp((x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14), 0.0, 1.0);
}

float ShadowCalculation(vec4 fragPosLightSpace, sampler2D shadowMap, vec3 norm, vec3 lightDir) {
    // perspective divide (not needed for ortho but good practice)
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // outside light frustum = no shadow
    if (projCoords.z > 1.0) return 0.0;

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    // bias to avoid shadow acne
    float bias = max(0.005 * (1.0 - dot(norm, lightDir)), 0.0005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 12.0;

    return shadow;
}

vec3 CalculateLighting(vec3 norm, vec3 fragPos, LightGrid grid) {
    vec3 totalLighting = vec3(0.0);

    for (int i = 0; i < u_LightCounts.x; i++) {
        vec3 lightDir = normalize(-u_DirLights[i].direction.xyz);
        float diff = max(dot(norm, lightDir), 0.0);

        vec4 fragPosLightSpace = u_LightSpaceMatrix[i] * vec4(fragPos, 1.0);
        float shadow = ShadowCalculation(fragPosLightSpace, u_ShadowMap[i], norm, lightDir);

        totalLighting += (1.0 - shadow) * diff * u_DirLights[i].color.rgb * u_DirLights[i].color.w;
    }

    for (uint i = 0; i < grid.pointCount; i++) {
        uint lightIndex = globalLightIndexList[grid.offset + i];
        PointLight light = pointLights[lightIndex];

        vec3 lightDir = normalize(light.position.xyz - fragPos);
        float distance = length(light.position.xyz - fragPos);

        if (distance > light.position.w) continue;

        float diff = max(dot(norm, lightDir), 0.0);
        float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));
        totalLighting += diff * light.color.rgb * light.color.w * attenuation;
    }

    for (uint i = 0; i < grid.spotCount; i++) {
        uint lightIndex = globalLightIndexList[grid.offset + grid.pointCount + i];
        SpotLight light = spotLights[lightIndex];

        vec3 lightDir = normalize(light.position.xyz - fragPos);
        float distance = length(light.position.xyz - fragPos);

        if (distance > light.position.w) continue;

        float diff = max(dot(norm, lightDir), 0.0);
        float attenuation = 1.0 / (1.0 + light.params.z * distance + light.params.w * (distance * distance));

        float theta = dot(lightDir, normalize(-light.direction.xyz));
        float epsilon = light.params.x - light.params.y;
        float intensityFactor = clamp((theta - light.params.y) / epsilon, 0.0, 1.0);

        totalLighting += diff * light.color.rgb * light.color.w * attenuation * intensityFactor;
    }

    return totalLighting;
}

#endif