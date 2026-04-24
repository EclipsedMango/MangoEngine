
#ifndef MANGORENDERING_RAYCASTNODE3D_H
#define MANGORENDERING_RAYCASTNODE3D_H

#include "Node3d.h"

class RigidBody3d;

class RayCastNode3d : public Node3d {
public:
    RayCastNode3d();
    ~RayCastNode3d() override = default;

    [[nodiscard]] std::unique_ptr<Node3d> Clone() override;
    void PhysicsProcess(float deltaTime) override;

    [[nodiscard]] std::string GetNodeType() const override { return "RayCastNode3d"; }

    void SetEnabled(const bool enabled) { m_enabled = enabled; }
    void SetTargetPosition(const glm::vec3& targetPosition) { m_targetPosition = targetPosition; }
    void SetExcludeParentBody(const bool exclude) { m_excludeParentBody = exclude; }

    [[nodiscard]] bool IsEnabled() const { return m_enabled; }
    [[nodiscard]] glm::vec3 GetTargetPosition() const { return m_targetPosition; }
    [[nodiscard]] bool GetExcludeParentBody() const { return m_excludeParentBody; }

    void ForceUpdate();

    [[nodiscard]] bool IsColliding() const { return m_colliding; }
    [[nodiscard]] glm::vec3 GetCollisionPoint() const { return m_collisionPoint; }
    [[nodiscard]] glm::vec3 GetCollisionNormal() const { return m_collisionNormal; }
    [[nodiscard]] float GetCollisionFraction() const { return m_collisionFraction; }

    [[nodiscard]] RigidBody3d* GetCollider() const { return m_collider; }

private:
    void ClearHit();
    [[nodiscard]] RigidBody3d* FindAncestorRigidBody() const;
    [[nodiscard]] glm::mat4 ComposeWorldMatrix() const;

    bool m_enabled = true;

    // local-space ray end point
    // ray starts at this node's origin and ends at target_position
    glm::vec3 m_targetPosition = glm::vec3(0.0f, -1.0f, 0.0f);

    bool m_excludeParentBody = true;
    bool m_colliding = false;
    glm::vec3 m_collisionPoint = glm::vec3(0.0f);
    glm::vec3 m_collisionNormal = glm::vec3(0.0f, 1.0f, 0.0f);
    float m_collisionFraction = 0.0f;
    RigidBody3d* m_collider = nullptr;

};



#endif //MANGORENDERING_RAYCASTNODE3D_H
