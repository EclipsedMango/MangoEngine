//
// Created by eclipsedmango on 27/3/26.
//

#include "DebugDraw.h"

void DebugDraw::DrawWorldOBB(const glm::mat4 &viewProj, const glm::mat4 &worldMatrix, const glm::vec3 &localCenter, const glm::vec3 &half, ImU32 color, const ImVec2 &viewportPos, const ImVec2 &viewportSize) {
    ImDrawList* list = ImGui::GetWindowDrawList();

    const glm::vec3 lc[8] = {
        localCenter + glm::vec3(-half.x,-half.y,-half.z),
        localCenter + glm::vec3( half.x,-half.y,-half.z),
        localCenter + glm::vec3( half.x, half.y,-half.z),
        localCenter + glm::vec3(-half.x, half.y,-half.z),
        localCenter + glm::vec3(-half.x,-half.y, half.z),
        localCenter + glm::vec3( half.x,-half.y, half.z),
        localCenter + glm::vec3( half.x, half.y, half.z),
        localCenter + glm::vec3(-half.x, half.y, half.z),
    };

    constexpr int edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    ImVec2 screen[8];
    bool visible[8];
    for (int i = 0; i < 8; i++) {
        const glm::vec3 worldCorner = glm::vec3(worldMatrix * glm::vec4(lc[i], 1.0f));
        visible[i] = WorldToScreen(worldCorner, viewProj, viewportPos, viewportSize, screen[i]);
    }

    for (const auto& edge : edges) {
        if (visible[edge[0]] && visible[edge[1]]) {
            list->AddLine(screen[edge[0]], screen[edge[1]], color, 1.75f);
        }
    }
}

bool DebugDraw::WorldToScreen(const glm::vec3 &worldPos, const glm::mat4 &viewProj, const ImVec2 &viewportPos, const ImVec2 &viewportSize, ImVec2 &out) {
    const glm::vec4 clip = viewProj * glm::vec4(worldPos, 1.0f);

    if (clip.w <= 0.0f) {
        return false;
    }

    const glm::vec3 ndc = glm::vec3(clip) / clip.w;

    out.x = viewportPos.x + (ndc.x * 0.5f + 0.5f) * viewportSize.x;
    out.y = viewportPos.y + (1.0f - (ndc.y * 0.5f + 0.5f)) * viewportSize.y;

    return true;
}

void DebugDraw::ExpandLocalAABB(Node3d *root, const Node3d *node, const glm::mat4 &rootWorldInv, glm::vec3 &minB, glm::vec3 &maxB) {
    constexpr glm::vec3 localCorners[8] = {
        {-0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f},
        {-0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f},
    };

    for (const auto& c : localCorners) {
        const glm::vec3 worldCorner = glm::vec3(node->GetWorldMatrix() * glm::vec4(c, 1.0f));
        const glm::vec3 rootLocal = glm::vec3(rootWorldInv * glm::vec4(worldCorner, 1.0f));
        minB = glm::min(minB, rootLocal);
        maxB = glm::max(maxB, rootLocal);
    }

    for (const auto child : node->GetChildren()) {
        ExpandLocalAABB(root, child, rootWorldInv, minB, maxB);
    }
}

void DebugDraw::DrawWorldDirections() {
}
