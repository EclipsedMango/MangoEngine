
#include "EditorCameraController.h"

#include "imgui.h"
#include "Core/Input.h"
#include "Core/Window.h"
#include "Nodes/CameraNode3d.h"

EditorCameraController::EditorCameraController(CameraNode3d *camera, Window *window) : m_camera(camera), m_window(window) {}

void EditorCameraController::Update(const float deltaTime, const bool viewportHovered) {
    if (!m_camera) return;

    const ImGuiIO& io = ImGui::GetIO();
    const bool imguiWantsKeys = io.WantCaptureKeyboard;

    const bool rmbHeld = Input::IsMouseButtonHeld(SDL_BUTTON_RIGHT);
    const bool shouldLook = rmbHeld && viewportHovered;

    if (shouldLook && !m_rmbLook) {
        m_rmbLook = true;
        SDL_SetWindowRelativeMouseMode(m_window->GetSDLWindow(), true);
        Input::SetMouseDeltaEnabled(true);
    } else if (!shouldLook && m_rmbLook) {
        CancelLook();
    }

    if (m_rmbLook && viewportHovered) {
        const float wheelY = Input::GetMouseWheelY();
        if (wheelY != 0.0f) {
            m_moveSpeed *= std::pow(1.1f, wheelY);
            m_moveSpeed = glm::clamp(m_moveSpeed, 0.05f, 500.0f);
        }
    }

    if (!m_rmbLook) return;

    const glm::vec2 md = Input::GetMouseDelta();
    m_camera->Rotate(md.x * m_mouseSensitivity, -md.y * m_mouseSensitivity);

    if (!imguiWantsKeys) {
        float speed = m_moveSpeed;
        if (Input::IsKeyHeld(SDL_SCANCODE_LSHIFT)) speed *= 5.0f;

        glm::vec3 dir(0.0f);
        if (Input::IsKeyHeld(SDL_SCANCODE_W)) dir += m_camera->GetFront();
        if (Input::IsKeyHeld(SDL_SCANCODE_S)) dir -= m_camera->GetFront();
        if (Input::IsKeyHeld(SDL_SCANCODE_D)) dir += m_camera->GetRight();
        if (Input::IsKeyHeld(SDL_SCANCODE_A)) dir -= m_camera->GetRight();
        if (Input::IsKeyHeld(SDL_SCANCODE_E)) dir += glm::vec3(0, 1, 0);
        if (Input::IsKeyHeld(SDL_SCANCODE_Q)) dir -= glm::vec3(0, 1, 0);

        if (glm::length(dir) > 0.0f) {
            dir = glm::normalize(dir);
            m_camera->SetPosition(m_camera->GetPosition() + dir * speed * deltaTime);
        }
    }
}

void EditorCameraController::CancelLook() {
    if (!m_rmbLook) {
        return;
    }

    m_rmbLook = false;
    SDL_SetWindowRelativeMouseMode(m_window->GetSDLWindow(), false);
    Input::SetMouseDeltaEnabled(false);
}
