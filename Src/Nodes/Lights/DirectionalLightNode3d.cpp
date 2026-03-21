
#include "DirectionalLightNode3d.h"

DirectionalLightNode3d::DirectionalLightNode3d(const glm::vec3 direction, const glm::vec3 color, const float intensity) : LightNode3d(color, intensity), m_light(direction, color, intensity) {
    glm::vec3 up = fabsf(direction.y) > 0.999f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
    SetRotation(glm::quatLookAt(glm::normalize(direction), up));
    SetName("DirectionalLightNode3d");
    AddProperty("intensity",
        [this]{ return GetIntensity(); },
        [this](const PropertyValue& v) { SetIntensity(std::get<float>(v)); }
    );
}

void DirectionalLightNode3d::Process(float deltaTime) {
    m_light.SetColor(GetColor());
    m_light.SetIntensity(GetIntensity());
}

void DirectionalLightNode3d::SyncLight() {
    const glm::vec3 dir = glm::normalize(glm::vec3(GetWorldMatrix() * glm::vec4(0, 0, -1, 0)));
    m_light.SetDirection(dir);
    m_light.SetColor(GetColor());
    m_light.SetIntensity(GetIntensity());
}
