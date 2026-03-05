
#ifndef MANGORENDERING_SHADOWMAP_H
#define MANGORENDERING_SHADOWMAP_H

#include <memory>
#include <glm/glm.hpp>
#include "Buffers/Framebuffer.h"
#include "Shader.h"

class ShadowMap {
public:
    ShadowMap(uint32_t width, uint32_t height);
    ~ShadowMap() = default;

    ShadowMap(const ShadowMap&) = delete;
    ShadowMap& operator=(const ShadowMap&) = delete;

    void BeginRender() const;
    static void EndRender();

    void SetLightDirection(const glm::vec3& direction);
    void SetProjectionBounds(float size, float nearPlane, float farPlane, glm::vec3 center);

    [[nodiscard]] glm::mat4 GetLightSpaceMatrix() const { return m_lightSpaceMatrix; }
    [[nodiscard]] uint32_t GetDepthTexture() const { return m_framebuffer->GetDepthAttachment(); }
    [[nodiscard]] uint32_t GetWidth() const { return m_framebuffer->GetWidth(); }
    [[nodiscard]] uint32_t GetHeight() const { return m_framebuffer->GetHeight(); }

private:
    void UpdateLightSpaceMatrix();

    std::unique_ptr<Framebuffer> m_framebuffer;

    glm::vec3 m_lightDirection = glm::vec3(-1.0f, -1.0f, -1.0f);
    float m_projectionSize = 20.0f;
    float m_nearPlane = 0.1f;
    float m_farPlane = 100.0f;
    glm::vec3 m_center = glm::vec3(0.0f);

    glm::mat4 m_lightSpaceMatrix = glm::mat4(1.0f);
};


#endif //MANGORENDERING_SHADOWMAP_H