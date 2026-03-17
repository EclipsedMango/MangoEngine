
#include "CameraNode3d.h"

#include "Core/Input.h"

CameraNode3d::CameraNode3d(const glm::vec3 position, const float fov, const float aspectRatio) : m_fov(fov), m_aspectRatio(aspectRatio) {
    SetPosition(position);
    UpdateVectors();
    SetName("CameraNode3d");
}

glm::mat4 CameraNode3d::GetViewMatrix() const {
    return glm::lookAt(GetPosition(), GetPosition() + m_front, m_up);
}

glm::mat4 CameraNode3d::GetProjectionMatrix() const {
    return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
}

void CameraNode3d::Rotate(const float yawDelta, const float pitchDelta) {
    m_yaw += yawDelta;
    m_pitch = glm::clamp(m_pitch + pitchDelta, -89.0f, 89.0f);
    UpdateVectors();
}

void CameraNode3d::UpdateVectors() {
    const glm::vec3 front = {
        cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch)),
        sin(glm::radians(m_pitch)),
        sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch))
    };

    m_front = glm::normalize(front);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up    = glm::normalize(glm::cross(m_right, m_front));
}