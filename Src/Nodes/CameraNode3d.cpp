
#include "CameraNode3d.h"

#include "Core/Input.h"

CameraNode3d::CameraNode3d(const glm::vec3 position, const float fov, const float aspectRatio) : m_fov(fov), m_aspectRatio(aspectRatio) {
    SetPosition(position);
    UpdateVectors();
}

void CameraNode3d::Process(float deltaTime) {
    float speed = m_moveSpeed;
    if (Input::IsKeyHeld(SDL_SCANCODE_LSHIFT)) speed += m_fastSpeed;
    if (Input::IsKeyHeld(SDL_SCANCODE_W)) SetPosition(GetPosition() + m_front * speed * deltaTime);
    if (Input::IsKeyHeld(SDL_SCANCODE_A)) SetPosition(GetPosition() + -m_right * speed * deltaTime);
    if (Input::IsKeyHeld(SDL_SCANCODE_S)) SetPosition(GetPosition() + -m_front * speed * deltaTime);
    if (Input::IsKeyHeld(SDL_SCANCODE_D)) SetPosition(GetPosition() + m_right * speed * deltaTime);

    const glm::vec2 mouseDelta = Input::GetMouseDelta();
    if (mouseDelta.x != 0 || mouseDelta.y != 0) {
        Rotate(mouseDelta.x * m_mouseSensitivity, -mouseDelta.y * m_mouseSensitivity);
    }
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