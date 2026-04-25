
#ifndef MANGORENDERING_PORTALNODE3D_H
#define MANGORENDERING_PORTALNODE3D_H

#include <unordered_map>
#include "MeshNode3d.h"

class AreaNode3d;
class CameraNode3d;

class PortalNode3d : public MeshNode3d {
public:
    PortalNode3d();
    ~PortalNode3d() override;

    [[nodiscard]] std::unique_ptr<Node3d> Clone() override;

    void PhysicsProcess(float deltaTime) override;

    void LinkTo(PortalNode3d* other);
    void Unlink();

    static void LinkPair(PortalNode3d* a, PortalNode3d* b);

    [[nodiscard]] PortalNode3d* GetLinkedPortal() const { return m_linkedPortal; }
    [[nodiscard]] bool IsLinked() const { return m_linkedPortal != nullptr; }

    [[nodiscard]] std::string GetNodeType() const override { return "PortalNode3d"; }

private:
    void Init() override;

    [[nodiscard]] AreaNode3d* GetAreaChild() const;

    void EnsureAreaSignalConnection();
    void DisconnectAreaSignalConnection();

    void OnAreaBodyEntered(const std::vector<SignalArg>& args);
    void OnAreaBodyExited(const std::vector<SignalArg>& args);

    void ProcessTeleportCandidates(float deltaTime);
    void TeleportNode(Node3d* node) const;

    [[nodiscard]] float GetSignedPortalSide(Node3d* node) const;

    [[nodiscard]] static glm::mat4 ComposeWorldMatrix(Node3d* node);
    [[nodiscard]] static glm::mat4 GetTeleportableWorldMatrix(Node3d* node);
    [[nodiscard]] static glm::mat4 GetThroughPortalMatrix(const glm::mat4& sourcePortalWorld, const glm::mat4& destinationPortalWorld);

    PortalNode3d* m_linkedPortal = nullptr;
    std::string m_linkedPortalName;

    AreaNode3d* m_connectedArea = nullptr;

    /*
        stores the previous signed distance of overlapping bodies relative to
        this portal's local Z plane

        positive local Z is considered the front side of the portal,
        teleport happens when a body moves from positive Z to zero/negative Z.
    */
    std::unordered_map<Node3d*, float> m_candidatePreviousSide;
    std::unordered_map<Node3d*, float> m_teleportCooldowns;
    float m_teleportCooldownSeconds = 0.15f;
};


#endif //MANGORENDERING_PORTALNODE3D_H