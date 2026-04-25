
#ifndef MANGORENDERING_AREANODE3D_H
#define MANGORENDERING_AREANODE3D_H

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

#include "Node3d.h"

class CollisionShape3d;

class AreaNode3d : public Node3d {
public:
    AreaNode3d();
    ~AreaNode3d() override;

    [[nodiscard]] std::unique_ptr<Node3d> Clone() override;

    void PhysicsProcess(float deltaTime) override;

    void SetEnabled(bool enabled);
    [[nodiscard]] bool IsEnabled() const { return m_enabled; }

    [[nodiscard]] bool HasPhysicsBody() const { return m_hasBody; }
    [[nodiscard]] JPH::BodyID GetBodyId() const { return m_bodyId; }

    [[nodiscard]] bool HasOverlappingBodies() const;
    [[nodiscard]] size_t GetOverlappingBodyCount() const;
    [[nodiscard]] Node3d* GetOverlappingBody(size_t index) const;
    [[nodiscard]] std::vector<Node3d*> GetOverlappingBodies() const;
    [[nodiscard]] bool IsBodyOverlapping(const Node3d* node) const;

    [[nodiscard]] std::string GetNodeType() const override { return "AreaNode3d"; }

    /*
        called by the physics contact listener
        do not normally call these manually
    */
    void NotifyBodyEntered(JPH::BodyID bodyId, Node3d* node);
    void NotifyBodyExited(JPH::BodyID bodyId);

private:
    void EnsureBody();
    void RecreateBody();
    void DestroyBody();
    void PushTransformToPhysics() const;

    [[nodiscard]] CollisionShape3d* GetCollisionShapeChild() const;

    static uint32_t BodyKey(JPH::BodyID bodyId);

    bool m_enabled = true;

    bool m_pendingRecreate = true;
    bool m_missingShapeWarned = false;
    bool m_hasBody = false;

    CollisionShape3d* m_cachedShapeNode = nullptr;
    std::string m_cachedShapeType = "Box";
    glm::vec3 m_cachedShapeSize = glm::vec3(0.5f);

    JPH::BodyID m_bodyId {};

    mutable std::mutex m_overlapMutex;
    std::unordered_map<uint32_t, Node3d*> m_overlappingBodies;
};

#endif //MANGORENDERING_AREANODE3D_H
