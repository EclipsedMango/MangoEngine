
#include "GizmoSystem.h"

#include "Core/Input.h"
#include "glm/gtc/type_ptr.hpp"
#include "Nodes/CameraNode3d.h"

void GizmoSystem::HandleShortcuts(bool isCameraLooking) {
    if (isCameraLooking) return;

    if (Input::IsKeyJustPressed(SDL_SCANCODE_W)) m_gizmoOp = ImGuizmo::TRANSLATE;
    if (Input::IsKeyJustPressed(SDL_SCANCODE_E)) m_gizmoOp = ImGuizmo::ROTATE;
    if (Input::IsKeyJustPressed(SDL_SCANCODE_R)) m_gizmoOp = ImGuizmo::SCALE;
}

bool GizmoSystem::UpdateAndDraw(const CameraNode3d* camera, const std::vector<Node3d*>& selectedNodes, const ImVec2& viewportPos, const ImVec2& viewportSize, const bool isPlaying, const bool isCameraLooking) {
    ImGuizmo::BeginFrame();

    if (selectedNodes.empty() || isPlaying || !camera) return false;

    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(viewportPos.x, viewportPos.y, viewportSize.x, viewportSize.y);
    ImGuizmo::Enable(!isCameraLooking);
    ImGuizmo::SetGizmoSizeClipSpace(0.2f);

    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 proj = camera->GetProjectionMatrix();

    glm::vec3 pivot(0.0f);
    for (const auto node : selectedNodes) {
        pivot += glm::vec3(node->GetWorldMatrix()[3]);
    }
    pivot /= static_cast<float>(selectedNodes.size());

    glm::mat4 pivotWorld = glm::translate(glm::mat4(1.0f), pivot);
    glm::mat4 delta = glm::mat4(1.0f);

    const bool snapObjectMovement = Input::IsKeyHeld(SDL_SCANCODE_LCTRL) && !Input::IsMouseButtonJustReleased(SDL_SCANCODE_LCTRL);

    constexpr glm::vec3 scaleTranslationSnap = glm::vec3(0.5f);
    constexpr glm::vec3 rotationSnap = glm::vec3(45.0f);

    const glm::vec3* snap = nullptr;
    if (snapObjectMovement) {
        if (m_gizmoOp == ImGuizmo::TRANSLATE || m_gizmoOp == ImGuizmo::SCALE) {
            snap = &scaleTranslationSnap;
        } else if (m_gizmoOp == ImGuizmo::ROTATE) {
            snap = &rotationSnap;
        }
    }

    ImGuizmo::Manipulate(
        glm::value_ptr(view),
        glm::value_ptr(proj),
        m_gizmoOp,
        m_gizmoMode,
        glm::value_ptr(pivotWorld),
        glm::value_ptr(delta),
        snapObjectMovement ? glm::value_ptr(*snap) : nullptr
    );

    const bool isUsing = ImGuizmo::IsUsing();

    // snapshot scales the moment the user grabs the gizmo
    if (isUsing && !m_wasUsingGizmo) {
        m_gizmoInitialScales.clear();
        for (const auto node : selectedNodes) {
            m_gizmoInitialScales.push_back(node->GetScale());
        }
    }

    m_wasUsingGizmo = isUsing;

    if (!isUsing) return false;

    if (m_gizmoOp == ImGuizmo::SCALE) {
        const glm::vec3 totalScale = {
            glm::length(glm::vec3(pivotWorld[0])),
            glm::length(glm::vec3(pivotWorld[1])),
            glm::length(glm::vec3(pivotWorld[2]))
        };

        for (size_t i = 0; i < selectedNodes.size(); i++) {
            selectedNodes[i]->SetScale(m_gizmoInitialScales[i] * totalScale);
        }

        return true;
    }

    for (const auto node : selectedNodes) {
        glm::mat4 world = delta * node->GetWorldMatrix();

        glm::mat4 localMatrix = world;
        if (const Node3d* parent = node->GetParent()) {
            localMatrix = glm::inverse(parent->GetWorldMatrix()) * world;
        }

        const glm::vec3 scale = {
            glm::length(glm::vec3(localMatrix[0])),
            glm::length(glm::vec3(localMatrix[1])),
            glm::length(glm::vec3(localMatrix[2]))
        };

        constexpr float eps = 1e-6f;
        glm::mat3 rotMat = {
            scale.x > eps ? glm::vec3(localMatrix[0]) / scale.x : glm::vec3(1,0,0),
            scale.y > eps ? glm::vec3(localMatrix[1]) / scale.y : glm::vec3(0,1,0),
            scale.z > eps ? glm::vec3(localMatrix[2]) / scale.z : glm::vec3(0,0,1)
        };

        if (std::abs(glm::determinant(rotMat) - 1.0f) >= 0.01f) {
            rotMat[0] = glm::normalize(rotMat[0]);
            rotMat[1] = glm::normalize(glm::cross(glm::vec3(rotMat[2]), glm::vec3(rotMat[0])));
            rotMat[2] = glm::normalize(glm::cross(glm::vec3(rotMat[0]), glm::vec3(rotMat[1])));
        }

        node->SetRotation(glm::quat_cast(rotMat));
        node->SetPosition(glm::vec3(localMatrix[3]));
        node->SetScale(scale);
    }

    return true;
}