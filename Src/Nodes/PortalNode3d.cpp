
#include "PortalNode3d.h"

#include "AreaNode3d.h"
#include "RigidBody3d.h"
#include "Core/SignalBus.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <ranges>
#include <vector>

REGISTER_NODE_TYPE(PortalNode3d)

namespace {
    constexpr float PORTAL_HALF_TURN_RADIANS = 3.14159265358979323846f;
}

PortalNode3d::PortalNode3d() {
    SetName("PortalNode");
    PortalNode3d::Init();
}

PortalNode3d::~PortalNode3d() {
    DisconnectAreaSignalConnection();
    Unlink();
}

std::unique_ptr<Node3d> PortalNode3d::Clone() {
    auto clone = std::make_unique<PortalNode3d>();
    clone->SetName(GetName());
    if (auto* mat = GetMaterialPtr().get()) {
        clone->SetMaterial(std::make_shared<Material>(*mat));
    }
    clone->SetMesh(GetMeshPtr());

    if (clone->GetActiveMaterial() && GetActiveMaterial()) {
        clone->GetActiveMaterial()->SetShader(GetActiveMaterial()->GetShader());
    }

    clone->m_linkedPortalName = m_linkedPortalName;

    CopyBaseStateTo(*clone);

    for (Node3d* child : GetChildren()) {
        clone->AddChild(child->Clone());
    }

    return clone;
}

void PortalNode3d::PhysicsProcess(const float deltaTime) {
    MeshNode3d::PhysicsProcess(deltaTime);
    EnsureAreaSignalConnection();
    ProcessTeleportCandidates(deltaTime);
}

void PortalNode3d::LinkTo(PortalNode3d *other) {
    if (other == this) {
        return;
    }

    if (m_linkedPortal == other) {
        return;
    }

    Unlink();

    m_linkedPortal = other;
    m_linkedPortalName = other ? other->GetName() : "";
    if (!other) {
        return;
    }

    if (other->m_linkedPortal && other->m_linkedPortal != this) {
        other->m_linkedPortal->m_linkedPortal = nullptr;
    }

    other->m_linkedPortal = this;
    other->m_linkedPortalName = GetName();
}

void PortalNode3d::Unlink() {
    if (!m_linkedPortal) {
        return;
    }

    PortalNode3d* other = m_linkedPortal;
    m_linkedPortal = nullptr;
    m_linkedPortalName = "";

    if (other->m_linkedPortal == this) {
        other->m_linkedPortal = nullptr;
        other->m_linkedPortalName = "";
    }
}

void PortalNode3d::LinkPair(PortalNode3d *a, PortalNode3d *b) {
    if (a) {
        a->LinkTo(b);
    } else if (b) {
        b->Unlink();
    }
}

void PortalNode3d::Init() {
    AddProperty("is_linked",
        [this]() -> PropertyValue { return m_linkedPortal != nullptr; },
        [](const PropertyValue&) {}
    );

    AddProperty("linked_portal_name",
        [this]() -> PropertyValue { return m_linkedPortalName; },
        [this](const PropertyValue& v) { m_linkedPortalName = std::get<std::string>(v); }
    );
}

AreaNode3d * PortalNode3d::GetAreaChild() const {
    for (Node3d* child : GetChildren()) {
        if (auto* area = dynamic_cast<AreaNode3d*>(child)) {
            return area;
        }
    }

    return nullptr;
}

void PortalNode3d::EnsureAreaSignalConnection() {
    AreaNode3d* area = GetAreaChild();

    if (area == m_connectedArea) {
        return;
    }

    DisconnectAreaSignalConnection();

    m_connectedArea = area;
    m_candidatePreviousSide.clear();

    if (!m_connectedArea) {
        return;
    }

    SignalBus::Get().ConnectNative(m_connectedArea, "body_entered", this,
        [this](const std::vector<SignalArg>& args) {
            OnAreaBodyEntered(args);
        }
    );

    SignalBus::Get().ConnectNative(m_connectedArea, "body_exited", this,
        [this](const std::vector<SignalArg>& args) {
            OnAreaBodyExited(args);
        }
    );

    // handle the case where an area already has overlapping bodies before the portal connects to its signals
    for (Node3d* body : m_connectedArea->GetOverlappingBodies()) {
        if (body) {
            m_candidatePreviousSide[body] = GetSignedPortalSide(body);
        }
    }
}

void PortalNode3d::DisconnectAreaSignalConnection() {
    if (!m_connectedArea) {
        return;
    }

    SignalBus::Get().DisconnectNative(
        m_connectedArea,
        "body_entered",
        this
    );

    SignalBus::Get().DisconnectNative(
        m_connectedArea,
        "body_exited",
        this
    );

    m_connectedArea = nullptr;
    m_candidatePreviousSide.clear();
}

void PortalNode3d::OnAreaBodyEntered(const std::vector<SignalArg> &args) {
    if (args.empty() || !std::holds_alternative<Node3d*>(args[0])) {
        return;
    }

    Node3d* body = std::get<Node3d*>(args[0]);
    if (!body || body == this || body == m_linkedPortal || body == m_connectedArea) {
        return;
    }

    m_candidatePreviousSide[body] = GetSignedPortalSide(body);
}

void PortalNode3d::OnAreaBodyExited(const std::vector<SignalArg> &args) {
    if (args.empty() || !std::holds_alternative<Node3d*>(args[0])) {
        return;
    }

    Node3d* body = std::get<Node3d*>(args[0]);
    if (!body) {
        return;
    }

    m_candidatePreviousSide.erase(body);
    m_teleportCooldowns.erase(body);
}

void PortalNode3d::ProcessTeleportCandidates(const float deltaTime) {
    for (auto it = m_teleportCooldowns.begin(); it != m_teleportCooldowns.end();) {
        it->second -= deltaTime;

        if (it->second <= 0.0f) {
            it = m_teleportCooldowns.erase(it);
        } else {
            ++it;
        }
    }

    if (!m_linkedPortal || !m_connectedArea || !m_connectedArea->IsEnabled()) {
        return;
    }

    for (auto it = m_candidatePreviousSide.begin(); it != m_candidatePreviousSide.end();) {
        Node3d* body = it->first;

        if (!body) {
            it = m_candidatePreviousSide.erase(it);
            continue;
        }

        const float previousSide = it->second;
        const float currentSide = GetSignedPortalSide(body);
        if (m_teleportCooldowns.contains(body)) {
            it->second = currentSide;
            ++it;
            continue;
        }

        /*
            assumes the portal's local +Z is the entry/front side,
            teleport when moving from +Z to -Z through the portal plane
        */
        const bool crossedPortal = previousSide > 0.0f && currentSide <= 0.0f;

        if (crossedPortal) {
            TeleportNode(body);

            m_teleportCooldowns[body] = m_teleportCooldownSeconds;

            if (m_linkedPortal) {
                m_linkedPortal->m_teleportCooldowns[body] = m_linkedPortal->m_teleportCooldownSeconds;
            }

            it = m_candidatePreviousSide.erase(it);
            continue;
        }

        it->second = currentSide;
        ++it;
    }
}

void PortalNode3d::TeleportNode(Node3d *node) const {
    if (!node || !m_linkedPortal) {
        return;
    }

    const glm::mat4 sourcePortalWorld = ComposeWorldMatrix(const_cast<PortalNode3d*>(this));
    const glm::mat4 destinationPortalWorld = ComposeWorldMatrix(m_linkedPortal);

    const glm::mat4 throughPortal = GetThroughPortalMatrix(sourcePortalWorld, destinationPortalWorld);

    const glm::mat4 oldNodeWorld = GetTeleportableWorldMatrix(node);
    const glm::mat4 newNodeWorld = throughPortal * oldNodeWorld;

    if (auto* rigidBody = dynamic_cast<RigidBody3d*>(node)) {
        const glm::quat sourcePortalRotation = glm::normalize(glm::quat_cast(sourcePortalWorld));
        const glm::quat destinationPortalRotation = glm::normalize(glm::quat_cast(destinationPortalWorld));

        const glm::quat halfTurn = glm::angleAxis(PORTAL_HALF_TURN_RADIANS, glm::vec3(0.0f, 1.0f, 0.0f));

        const glm::quat velocityRotation = destinationPortalRotation * halfTurn * glm::inverse(sourcePortalRotation);
        const glm::vec3 newLinearVelocity = velocityRotation * rigidBody->GetLinearVelocity();
        const glm::vec3 newAngularVelocity = velocityRotation * rigidBody->GetAngularVelocity();

        rigidBody->TeleportToWorldTransform(newNodeWorld);
        rigidBody->SetLinearVelocity(newLinearVelocity);
        rigidBody->SetAngularVelocity(newAngularVelocity);

        return;
    }

    glm::mat4 parentWorld(1.0f);

    if (Node3d* parent = node->GetParent()) {
        parentWorld = ComposeWorldMatrix(parent);
    }

    const glm::mat4 newLocal = glm::inverse(parentWorld) * newNodeWorld;
    node->SetLocalTransform(newLocal);
}

float PortalNode3d::GetSignedPortalSide(Node3d *node) const {
    if (!node) {
        return 0.0f;
    }

    const glm::mat4 portalWorld = ComposeWorldMatrix(const_cast<PortalNode3d*>(this));
    const glm::mat4 inversePortalWorld = glm::inverse(portalWorld);

    glm::mat4 nodeWorld;
    if (const auto* rigidBody = dynamic_cast<RigidBody3d*>(node)) {
        nodeWorld = rigidBody->GetPhysicsWorldTransform();
    } else {
        nodeWorld = ComposeWorldMatrix(node);
    }

    const glm::vec3 worldPosition = glm::vec3(nodeWorld[3]);
    const glm::vec3 localPosition = glm::vec3(inversePortalWorld * glm::vec4(worldPosition, 1.0f));

    return localPosition.z;
}

glm::mat4 PortalNode3d::ComposeWorldMatrix(Node3d *node) {
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

glm::mat4 PortalNode3d::GetTeleportableWorldMatrix(Node3d *node) {
    if (const auto* rigidBody = dynamic_cast<RigidBody3d*>(node)) {
        return rigidBody->GetPhysicsWorldTransform();
    }

    return ComposeWorldMatrix(node);
}

glm::mat4 PortalNode3d::GetThroughPortalMatrix(const glm::mat4 &sourcePortalWorld, const glm::mat4 &destinationPortalWorld) {
    /*
        move into source portal local space, rotate 180 degrees, then move into destination portal world space
        this makes walking into the front of portal A come out of the front of portal B
    */
    const glm::mat4 halfTurn =glm::rotate(glm::mat4(1.0f), PORTAL_HALF_TURN_RADIANS, glm::vec3(0.0f, 1.0f, 0.0f));
    return destinationPortalWorld * halfTurn * glm::inverse(sourcePortalWorld);
}
