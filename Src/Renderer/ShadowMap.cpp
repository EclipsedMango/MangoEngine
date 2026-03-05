
#include "ShadowMap.h"

#include <glm/gtc/matrix_transform.hpp>
#include "glad/gl.h"

ShadowMap::ShadowMap(const uint32_t width, const uint32_t height) {
    m_framebuffer = std::make_unique<Framebuffer>(width, height, FramebufferType::DepthOnly);
    UpdateLightSpaceMatrix();
}

void ShadowMap::BeginRender() const {
    m_framebuffer->Bind();
    glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMap::EndRender() {
    Framebuffer::Unbind();
}

void ShadowMap::SetLightDirection(const glm::vec3& direction) {
    m_lightDirection = glm::normalize(direction);
    UpdateLightSpaceMatrix();
}

void ShadowMap::SetProjectionBounds(const float size, const float nearPlane, const float farPlane, glm::vec3 center) {
    m_projectionSize = size;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    m_center = center;
    UpdateLightSpaceMatrix();
}

void ShadowMap::UpdateLightSpaceMatrix() {
    // orthographic projection for directional light
    const glm::mat4 projection = glm::ortho(
        -m_projectionSize, m_projectionSize,
        -m_projectionSize, m_projectionSize,
        m_nearPlane, m_farPlane
    );

    // position the light source back along its direction
    const glm::vec3 lightPos = m_center - m_lightDirection * (m_farPlane * 0.5f);
    const glm::mat4 view = glm::lookAt(
        lightPos,
        m_center,  // look at the scene center
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    m_lightSpaceMatrix = projection * view;
}