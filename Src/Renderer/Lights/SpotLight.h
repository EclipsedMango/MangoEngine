
#ifndef MANGORENDERING_SPOTLIGHT_H
#define MANGORENDERING_SPOTLIGHT_H

#include <glm/glm.hpp>

class SpotLight {
public:
    SpotLight(glm::vec3 position, glm::vec3 direction, glm::vec3 color, float intensity);

    void SetPosition(const glm::vec3 position) { m_position = position; }
    void SetDirection(const glm::vec3 direction) { m_direction = direction; }
    void SetColor(const glm::vec3 color) { m_color = color; }
    void SetIntensity(const float intensity) { m_intensity = intensity; }
    void SetRadius(const float radius) { m_radius = radius; }
    void SetCutOffs(float cutOffAngle, float outerCutOffAngle);

    [[nodiscard]] glm::vec3 GetPosition() const { return m_position; }
    [[nodiscard]] glm::vec3 GetDirection() const { return m_direction; }
    [[nodiscard]] glm::vec3 GetColor() const { return m_color; }
    [[nodiscard]] float GetIntensity() const { return m_intensity; }
    [[nodiscard]] float GetRadius() const { return m_radius; }
    [[nodiscard]] float GetCutOff() const { return m_cutOff; }
    [[nodiscard]] float GetOuterCutOff() const { return m_outerCutOff; }

private:
    glm::vec3 m_position{};
    glm::vec3 m_direction{};
    glm::vec3 m_color{};
    float m_intensity = 1.0f;
    float m_radius = 8.0f;
    float m_cutOff{};
    float m_outerCutOff{};
};


#endif //MANGORENDERING_SPOTLIGHT_H