
#include "RayCastNode3d.h"

#include <algorithm>
#include <vector>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/CollisionDispatch.h>

#include "Core/PhysicsWorld.h"
#include "RigidBody3d.h"

REGISTER_NODE_TYPE(RayCastNode3d)

namespace {
    JPH::RVec3 ToJoltRVec3(const glm::vec3& v) {
        return {v.x, v.y, v.z};
    }

    glm::vec3 ToGlmVec3(const JPH::Vec3& v) {
        return {v.GetX(), v.GetY(), v.GetZ()};
    }

    class IgnoreBodyFilter final : public JPH::BodyFilter {
    public:
        explicit IgnoreBodyFilter(const JPH::BodyID bodyToIgnore) : m_bodyToIgnore(bodyToIgnore) {}
        [[nodiscard]] bool ShouldCollide(const JPH::BodyID& bodyID) const override { return bodyID != m_bodyToIgnore; }
        [[nodiscard]] bool ShouldCollideLocked(const JPH::Body& body) const override { return body.GetID() != m_bodyToIgnore; }
    private:
        JPH::BodyID m_bodyToIgnore;
    };
}

RayCastNode3d::RayCastNode3d() {
    SetName("RayCastNode3d");

    AddProperty("enabled",
        [this]() -> PropertyValue { return m_enabled; },
        [this](const PropertyValue& v) { SetEnabled(std::get<bool>(v)); }
    );

    AddProperty("target_position",
        [this]() -> PropertyValue { return m_targetPosition; },
        [this](const PropertyValue& v) { SetTargetPosition(std::get<glm::vec3>(v)); }
    );

    AddProperty("exclude_parent_body",
        [this]() -> PropertyValue { return m_excludeParentBody; },
        [this](const PropertyValue& v) { SetExcludeParentBody(std::get<bool>(v)); }
    );
}

std::unique_ptr<Node3d> RayCastNode3d::Clone() {
    auto clone = std::make_unique<RayCastNode3d>();

    CopyBaseStateTo(*clone);

    clone->SetEnabled(m_enabled);
    clone->SetTargetPosition(m_targetPosition);
    clone->SetExcludeParentBody(m_excludeParentBody);

    for (Node3d* child : GetChildren()) {
        clone->AddChild(child->Clone());
    }

    return clone;
}

void RayCastNode3d::PhysicsProcess(const float deltaTime) {
    ForceUpdate();

    Node3d::PhysicsProcess(deltaTime);
}

void RayCastNode3d::ClearHit() {
    m_colliding = false;
    m_collisionPoint = glm::vec3(0.0f);
    m_collisionNormal = glm::vec3(0.0f, 1.0f, 0.0f);
    m_collisionFraction = 0.0f;
    m_collider = nullptr;
}

glm::mat4 RayCastNode3d::ComposeWorldMatrix() const {
    glm::mat4 world(1.0f);

    std::vector<const Node3d*> chain;

    for (const Node3d* cursor = this; cursor; cursor = cursor->GetParent()) {
        chain.push_back(cursor);
    }

    std::ranges::reverse(chain);
    for (const Node3d* node : chain) {
        world *= const_cast<Node3d*>(node)->GetLocalMatrix();
    }

    return world;
}

RigidBody3d* RayCastNode3d::FindAncestorRigidBody() const {
    for (Node3d* cursor = GetParent(); cursor; cursor = cursor->GetParent()) {
        if (auto* body = dynamic_cast<RigidBody3d*>(cursor)) {
            return body;
        }
    }

    return nullptr;
}

void RayCastNode3d::ForceUpdate() {
    ClearHit();

    if (!m_enabled) {
        return;
    }

    PhysicsWorld& physicsWorld = PhysicsWorld::Get();

    if (!physicsWorld.IsInitialized()) {
        return;
    }

    const glm::mat4 worldMatrix = ComposeWorldMatrix();

    const glm::vec3 origin = glm::vec3(worldMatrix[3]);
    const glm::vec3 target = glm::vec3(worldMatrix * glm::vec4(m_targetPosition, 1.0f));
    const glm::vec3 direction = target - origin;

    if (glm::length(direction) <= 0.00001f) {
        return;
    }

    const JPH::RRayCast ray(ToJoltRVec3(origin), JPH::Vec3(direction.x, direction.y, direction.z));
    JPH::RayCastResult result;

    const auto& system = physicsWorld.GetSystem();
    auto& query = system.GetNarrowPhaseQuery();

    bool hit = false;

    if (m_excludeParentBody) {
        const RigidBody3d* parentBody = FindAncestorRigidBody();

        if (parentBody && parentBody->HasPhysicsBody()) {
            const IgnoreBodyFilter filter(parentBody->GetBodyId());

            hit = query.CastRay(
                ray,
                result,
                JPH::BroadPhaseLayerFilter(),
                JPH::ObjectLayerFilter(),
                filter
            );
        } else {
            hit = query.CastRay(ray, result);
        }
    } else {
        hit = query.CastRay(ray, result);
    }

    if (!hit) {
        return;
    }

    m_colliding = true;
    m_collisionFraction = result.mFraction;
    m_collisionPoint = origin + direction * result.mFraction;

    const JPH::BodyLockRead lock(system.GetBodyLockInterface(), result.mBodyID);
    if (!lock.Succeeded()) {
        return;
    }

    const JPH::Body& body = lock.GetBody();

    m_collider = reinterpret_cast<RigidBody3d*>(body.GetUserData());

    const JPH::RVec3 hitPoint = ray.GetPointOnRay(result.mFraction);
    const JPH::Vec3 normal = body.GetWorldSpaceSurfaceNormal(result.mSubShapeID2, hitPoint);

    m_collisionNormal = ToGlmVec3(normal);
}
