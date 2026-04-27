
#include "ScriptManager.h"
#include "ScriptManagerInternal.h"

#include "Nodes/AreaNode3d.h"
#include "Nodes/CameraNode3d.h"
#include "Nodes/Node3d.h"
#include "Nodes/RayCastNode3d.h"
#include "Nodes/RigidBody3d.h"

void RegisterDerivedNodes(sol::state& lua) {
    lua.new_usertype<CameraNode3d>("CameraNode3d",
        sol::base_classes, sol::bases<Node3d, PropertyHolder>(),
        "Rotate", &CameraNode3d::Rotate,
        "GetFront", &CameraNode3d::GetFront,
        "GetRight", &CameraNode3d::GetRight,
        "GetUp", &CameraNode3d::GetUp
    );

    lua.new_usertype<RigidBody3d>("RigidBody3d",
        sol::base_classes, sol::bases<Node3d, PropertyHolder>(),
        "GetLinearVelocity",  &RigidBody3d::GetLinearVelocity,
        "SetLinearVelocity",  &RigidBody3d::SetLinearVelocity,
        "GetAngularVelocity", &RigidBody3d::GetAngularVelocity,
        "SetAngularVelocity", &RigidBody3d::SetAngularVelocity
    );

    lua.new_usertype<RayCastNode3d>("RayCastNode3d",
        sol::base_classes, sol::bases<Node3d, PropertyHolder>(),
        "SetEnabled", &RayCastNode3d::SetEnabled,
        "IsEnabled", &RayCastNode3d::IsEnabled,
        "SetTargetPosition", &RayCastNode3d::SetTargetPosition,
        "GetTargetPosition", &RayCastNode3d::GetTargetPosition,
        "SetExcludeParentBody", &RayCastNode3d::SetExcludeParentBody,
        "GetExcludeParentBody", &RayCastNode3d::GetExcludeParentBody,
        "ForceUpdate", &RayCastNode3d::ForceUpdate,
        "IsColliding", &RayCastNode3d::IsColliding,
        "GetCollisionPoint", &RayCastNode3d::GetCollisionPoint,
        "GetCollisionNormal", &RayCastNode3d::GetCollisionNormal,
        "GetCollisionFraction", &RayCastNode3d::GetCollisionFraction,
        "GetCollider", &RayCastNode3d::GetCollider
    );

    lua.new_usertype<AreaNode3d>("AreaNode3d",
        sol::base_classes, sol::bases<Node3d, PropertyHolder>(),
        "SetEnabled", &AreaNode3d::SetEnabled,
        "IsEnabled", &AreaNode3d::IsEnabled,
        "HasPhysicsBody", &AreaNode3d::HasPhysicsBody,
        "HasOverlappingBodies", &AreaNode3d::HasOverlappingBodies,
        "GetOverlappingBodyCount", &AreaNode3d::GetOverlappingBodyCount,
        "GetOverlappingBody", [](const AreaNode3d* area, const int index) -> Node3d* {
            if (!area || index < 1) return nullptr;
            return area->GetOverlappingBody(static_cast<size_t>(index - 1));
        },
        "IsBodyOverlapping", &AreaNode3d::IsBodyOverlapping
    );
}