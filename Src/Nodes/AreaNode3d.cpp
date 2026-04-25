
#include "AreaNode3d.h"

#include <algorithm>
#include <iostream>
#include <ranges>
#include <vector>

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/EActivation.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "CollisionShape3d.h"
#include "Core/NodeRegistry.h"
#include "Core/PhysicsWorld.h"

REGISTER_NODE_TYPE(AreaNode3d)

namespace {
    /*
        NON_MOVING vs NON_MOVING does not collide in filters

        AreaNode3d should detect both static and dynamic bodies, so putting it
        on MOVING makes it collide with both NON_MOVING and MOVING.
    */
    constexpr JPH::ObjectLayer LAYER_AREA = 1;

    JPH::Vec3 ToJoltVec3(const glm::vec3& vec) {
        return { vec.x, vec.y, vec.z };
    }

    JPH::Quat ToJoltQuat(const glm::quat& quat) {
        return { quat.x, quat.y, quat.z, quat.w };
    }

    glm::mat4 ComposeWorldMatrix(Node3d* node) {
        glm::mat4 world(1.0f);

        std::vector<Node3d*> chain;
        for (Node3d* cursor = node; cursor; cursor = cursor->GetParent()) {
            chain.push_back(cursor);
        }

        std::ranges::reverse(chain);

        for (Node3d* segment : chain) {
            world *= segment->GetLocalMatrix();
        }

        return world;
    }

    JPH::RefConst<JPH::Shape> BuildShapeFromNode(const CollisionShape3d& shapeNode) {
        const glm::vec3 shapeSize = glm::max(shapeNode.GetShapeSize(), glm::vec3(0.01f));

        switch (shapeNode.GetShapeType()) {
            case CollisionShape3d::ShapeType::Box: {
                return new JPH::BoxShape(ToJoltVec3(shapeSize));
            }
            case CollisionShape3d::ShapeType::Sphere: {
                const float radius = std::max(std::max(shapeSize.x, shapeSize.y), shapeSize.z);
                return new JPH::SphereShape(std::max(radius, 0.01f));
            }
            case CollisionShape3d::ShapeType::Capsule: {
                const float radius = std::max(shapeSize.x, 0.01f);
                const float halfHeight = std::max(shapeSize.y * 0.5f, 0.01f);
                return new JPH::CapsuleShape(halfHeight, radius);
            }
        }

        return new JPH::BoxShape(ToJoltVec3(shapeSize));
    }
}

AreaNode3d::AreaNode3d() {
    SetName("AreaNode3d");

    AddProperty("enabled",
        [this]() -> PropertyValue { return m_enabled; },
        [this](const PropertyValue& v) { SetEnabled(std::get<bool>(v)); }
    );
}

AreaNode3d::~AreaNode3d() {
    DestroyBody();
}


std::unique_ptr<Node3d> AreaNode3d::Clone() {
    auto clone = std::make_unique<AreaNode3d>();

    CopyBaseStateTo(*clone);
    clone->SetEnabled(m_enabled);
    clone->m_pendingRecreate = true;

    for (Node3d* child : GetChildren()) {
        clone->AddChild(child->Clone());
    }

    return clone;
}

void AreaNode3d::PhysicsProcess(const float deltaTime) {
    EnsureBody();

    /*
        if Lua moves the area in _physics_process, PushTransformToPhysics()
        below will apply the updated transform to the Jolt sensor body
    */
    Node3d::PhysicsProcess(deltaTime);

    if (m_hasBody && m_enabled) {
        PushTransformToPhysics();
    }
}


void AreaNode3d::SetEnabled(const bool enabled) {
    if (m_enabled == enabled) {
        return;
    }

    m_enabled = enabled;

    if (!m_enabled) {
        DestroyBody();
    } else {
        m_pendingRecreate = true;
    }
}

bool AreaNode3d::HasOverlappingBodies() const {
    std::lock_guard lock(m_overlapMutex);
    return !m_overlappingBodies.empty();
}

size_t AreaNode3d::GetOverlappingBodyCount() const {
    std::lock_guard lock(m_overlapMutex);
    return m_overlappingBodies.size();
}

Node3d* AreaNode3d::GetOverlappingBody(const size_t index) const {
    std::lock_guard lock(m_overlapMutex);

    if (index >= m_overlappingBodies.size()) {
        return nullptr;
    }

    size_t current = 0;

    for (const auto &node: m_overlappingBodies | std::views::values) {
        if (current == index) {
            return node;
        }

        ++current;
    }

    return nullptr;
}

std::vector<Node3d*> AreaNode3d::GetOverlappingBodies() const {
    std::lock_guard lock(m_overlapMutex);

    std::vector<Node3d*> result;
    result.reserve(m_overlappingBodies.size());

    for (const auto &node: m_overlappingBodies | std::views::values) {
        if (node) {
            result.push_back(node);
        }
    }

    return result;
}


bool AreaNode3d::IsBodyOverlapping(const Node3d* node) const {
    if (!node) {
        return false;
    }

    std::lock_guard lock(m_overlapMutex);

    for (const auto &overlappingNode: m_overlappingBodies | std::views::values) {
        if (overlappingNode == node) {
            return true;
        }
    }

    return false;
}

void AreaNode3d::NotifyBodyEntered(const JPH::BodyID bodyId, Node3d* node) {
    if (!node || node == this) {
        return;
    }

    bool inserted = false;

    {
        std::lock_guard lock(m_overlapMutex);
        const uint32_t key = BodyKey(bodyId);

        if (!m_overlappingBodies.contains(key)) {
            inserted = true;
        }

        m_overlappingBodies[key] = node;
    }

    if (inserted) {
        QueueSignal("body_entered", { node });
    }
}


void AreaNode3d::NotifyBodyExited(const JPH::BodyID bodyId) {
    Node3d* exitedNode = nullptr;

    {
        std::lock_guard lock(m_overlapMutex);

        const uint32_t key = BodyKey(bodyId);
        const auto it = m_overlappingBodies.find(key);

        if (it == m_overlappingBodies.end()) {
            return;
        }

        exitedNode = it->second;
        m_overlappingBodies.erase(it);
    }

    if (exitedNode) {
        QueueSignal("body_exited", { exitedNode });
    }
}

CollisionShape3d* AreaNode3d::GetCollisionShapeChild() const {
    for (Node3d* child : GetChildren()) {
        if (auto* shape = dynamic_cast<CollisionShape3d*>(child)) {
            return shape;
        }
    }

    return nullptr;
}

void AreaNode3d::EnsureBody() {
    if (!m_enabled) {
        DestroyBody();
        return;
    }

    const PhysicsWorld& world = PhysicsWorld::Get();
    if (!world.IsInitialized()) {
        return;
    }

    CollisionShape3d* shapeNode = GetCollisionShapeChild();

    if (!shapeNode) {
        if (!m_missingShapeWarned) {
            std::cerr << "[AreaNode3d Warning] '" << GetName() << "' has no CollisionShape3d child. Area body will not be created.\n";
            m_missingShapeWarned = true;
        }

        DestroyBody();
        m_cachedShapeNode = nullptr;
        return;
    }

    m_missingShapeWarned = false;

    const std::string shapeType = CollisionShape3d::ShapeTypeToString(shapeNode->GetShapeType());
    const glm::vec3 shapeSize = shapeNode->GetShapeSize();

    if (shapeNode != m_cachedShapeNode || shapeType != m_cachedShapeType || shapeSize != m_cachedShapeSize) {
        m_cachedShapeNode = shapeNode;
        m_cachedShapeType = shapeType;
        m_cachedShapeSize = shapeSize;
        m_pendingRecreate = true;
    }

    if (!m_hasBody || m_pendingRecreate) {
        RecreateBody();
    }
}

void AreaNode3d::RecreateBody() {
    PhysicsWorld& world = PhysicsWorld::Get();

    if (!world.IsInitialized()) {
        return;
    }

    CollisionShape3d* shapeNode = GetCollisionShapeChild();

    if (!shapeNode) {
        DestroyBody();
        return;
    }

    DestroyBody();

    const JPH::RefConst<JPH::Shape> shape = BuildShapeFromNode(*shapeNode);

    const glm::mat4 worldMatrix = ComposeWorldMatrix(this);
    const glm::vec3 worldPos = glm::vec3(worldMatrix[3]);
    const glm::quat worldRot = glm::normalize(glm::quat_cast(worldMatrix));

    JPH::BodyCreationSettings settings(
        shape.GetPtr(),
        JPH::RVec3(worldPos.x, worldPos.y, worldPos.z),
        ToJoltQuat(worldRot),
        JPH::EMotionType::Kinematic,
        LAYER_AREA
    );

    settings.mUserData = reinterpret_cast<JPH::uint64>(this);
    settings.mIsSensor = true;
    settings.mAllowDynamicOrKinematic = true;

    JPH::BodyInterface& bodyInterface = world.GetBodyInterface();

    if (const JPH::Body* body = bodyInterface.CreateBody(settings)) {
        m_bodyId = body->GetID();

        bodyInterface.AddBody(m_bodyId, JPH::EActivation::Activate);

        m_hasBody = true;
        m_pendingRecreate = false;
    }
}

void AreaNode3d::DestroyBody() {
    {
        std::lock_guard lock(m_overlapMutex);
        m_overlappingBodies.clear();
    }

    if (!m_hasBody) {
        return;
    }

    PhysicsWorld& world = PhysicsWorld::Get();

    if (world.IsInitialized()) {
        JPH::BodyInterface& bodyInterface = world.GetBodyInterface();

        bodyInterface.RemoveBody(m_bodyId);
        bodyInterface.DestroyBody(m_bodyId);
    }

    m_hasBody = false;
    m_bodyId = JPH::BodyID();
}

void AreaNode3d::PushTransformToPhysics() const {
    if (!m_hasBody) {
        return;
    }

    PhysicsWorld& world = PhysicsWorld::Get();
    JPH::BodyInterface& bodyInterface = world.GetBodyInterface();

    const glm::mat4 worldMatrix = ComposeWorldMatrix(const_cast<AreaNode3d*>(this));
    const glm::vec3 worldPos = glm::vec3(worldMatrix[3]);
    const glm::quat worldRot = glm::normalize(glm::quat_cast(worldMatrix));

    bodyInterface.SetPositionAndRotation(
        m_bodyId,
        JPH::RVec3(worldPos.x, worldPos.y, worldPos.z),
        ToJoltQuat(worldRot),
        JPH::EActivation::Activate
    );
}

uint32_t AreaNode3d::BodyKey(const JPH::BodyID bodyId) {
    return bodyId.GetIndexAndSequenceNumber();
}
