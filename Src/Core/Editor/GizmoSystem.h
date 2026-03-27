
#ifndef MANGORENDERING_GIZMOSYSTEM_H
#define MANGORENDERING_GIZMOSYSTEM_H

#include "imgui.h"
#include "ImGuizmo.h"
#include "glm/glm.hpp"
#include <vector>

class CameraNode3d;
class Node3d;

class GizmoSystem {
public:
    GizmoSystem() = default;

    void HandleShortcuts(bool isCameraLooking);
    void UpdateAndDraw(const CameraNode3d* camera, const std::vector<Node3d*>& selectedNodes, const ImVec2& viewportPos, const ImVec2& viewportSize, bool isPlaying, bool isCameraLooking);

private:
    ImGuizmo::OPERATION m_gizmoOp = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE m_gizmoMode = ImGuizmo::WORLD;

    bool m_wasUsingGizmo = false;
    std::vector<glm::vec3> m_gizmoInitialScales;
};


#endif //MANGORENDERING_GIZMOSYSTEM_H