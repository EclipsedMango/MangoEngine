
#ifndef MANGORENDERING_DEBUGDRAW_H
#define MANGORENDERING_DEBUGDRAW_H

#include "imgui.h"
#include "glm/glm.hpp"
#include "Nodes/Node3d.h"

class DebugDraw {
public:
    static void DrawWorldOBB(const glm::mat4& viewProj, const glm::mat4& worldMatrix, const glm::vec3& localCenter, const glm::vec3& half, ImU32 color, const ImVec2& viewportPos, const ImVec2& viewportSize);
    static bool WorldToScreen(const glm::vec3& worldPos, const glm::mat4& viewProj, const ImVec2& viewportPos, const ImVec2& viewportSize, ImVec2& out);
    static void ExpandLocalAABB(Node3d* root, const Node3d* node, const glm::mat4& rootWorldInv, glm::vec3& minB, glm::vec3& maxB);
    static void DrawWorldDirections();
};


#endif //MANGORENDERING_DEBUGDRAW_H