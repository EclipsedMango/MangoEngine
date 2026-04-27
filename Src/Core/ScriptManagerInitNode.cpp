
#include "ScriptManager.h"
#include "ScriptManagerInternal.h"

#include <iostream>

#include "Nodes/AreaNode3d.h"
#include "Nodes/CameraNode3d.h"
#include "Nodes/Node3d.h"
#include "Nodes/RayCastNode3d.h"
#include "Nodes/RigidBody3d.h"

void RegisterNode3d(sol::state& lua) {
    lua.new_usertype<Node3d>("Node3d",
        sol::base_classes, sol::bases<PropertyHolder>(),
        "GetName", &Node3d::GetName,
        "SetName", &Node3d::SetName,
        "GetPosition", &Node3d::GetPosition,
        "GetWorldPosition", &Node3d::GetWorldPosition,
        "SetPosition", &Node3d::SetPosition,
        "GetRotationEuler", &Node3d::GetRotationEuler,
        "SetRotationEuler", &Node3d::SetRotationEuler,
        "GetScale", &Node3d::GetScale,
        "SetScale", &Node3d::SetScale,
        "IsVisible", &Node3d::IsVisible,
        "SetVisible", &Node3d::SetVisible,
        "GetParent", &Node3d::GetParent,
        "GetNodeType", &Node3d::GetNodeType,
        "GetChildCount", &Node3d::GetChildCount,

        "GetChild", [](const Node3d* node, const int index) -> Node3d* {
            if (!node || index < 1) return nullptr;
            return node->GetChild(static_cast<size_t>(index - 1));
        },
        "FindChild", [](const Node3d* node, const std::string& name, const sol::optional<bool> recursive) -> Node3d* {
            if (!node) return nullptr;
            return node->FindChild(name, recursive.value_or(true));
        },
        "FindChildByType", [](const Node3d* node, const std::string& type, const sol::optional<bool> recursive) -> Node3d* {
            if (!node) return nullptr;
            return node->FindChildByType(type, recursive.value_or(true));
        },

        "AsCamera", [](Node3d* n) -> CameraNode3d* { return dynamic_cast<CameraNode3d*>(n); },
        "AsRigidBody", [](Node3d* n) -> RigidBody3d* { return dynamic_cast<RigidBody3d*>(n); },
        "AsRayCast", [](Node3d* n) -> RayCastNode3d* { return dynamic_cast<RayCastNode3d*>(n); },
        "AsArea", [](Node3d* n) -> AreaNode3d* { return dynamic_cast<AreaNode3d*>(n); },

        "Connect", [](Node3d* src, const std::string& sig, Node3d* tgt, const std::string& method) {
            if (src && tgt) src->Connect(sig, tgt, method);
        },
        "ConnectOneShot", [](Node3d* src, const std::string& sig, Node3d* tgt, const std::string& method) {
            if (src && tgt) src->Connect(sig, tgt, method, true);
        },
        "Disconnect", [](Node3d* src, const std::string& sig, Node3d* tgt, const std::string& method) {
            if (src && tgt) src->Disconnect(sig, tgt, method);
        },

        "EmitSignal", [](Node3d* src, const std::string& sig, sol::variadic_args va) {
            if (!src) return;
            std::vector<SignalArg> args;
            for (const sol::object& obj : va) {
                if (obj.is<bool>()) args.emplace_back(obj.as<bool>());
                else if (obj.is<int>()) args.emplace_back(obj.as<int>());
                else if (obj.is<double>()) args.emplace_back(static_cast<float>(obj.as<double>()));
                else if (obj.is<std::string>()) args.emplace_back(obj.as<std::string>());
                else if (obj.is<glm::vec2>()) args.emplace_back(obj.as<glm::vec2>());
                else if (obj.is<glm::vec3>()) args.emplace_back(obj.as<glm::vec3>());
                else if (obj.is<Node3d*>()) args.emplace_back(obj.as<Node3d*>());
                else std::cerr << "[Lua Signal Warning] Unsupported arg type for '" << sig << "'.\n";
            }
            src->EmitSignal(sig, args);
        },

        "QueueSignal", [](Node3d* src, const std::string& sig, sol::variadic_args va) {
            if (!src) return;
            std::vector<SignalArg> args;
            for (const sol::object& obj : va) {
                if (obj.is<bool>()) args.emplace_back(obj.as<bool>());
                else if (obj.is<int>()) args.emplace_back(obj.as<int>());
                else if (obj.is<double>()) args.emplace_back(static_cast<float>(obj.as<double>()));
                else if (obj.is<std::string>()) args.emplace_back(obj.as<std::string>());
                else if (obj.is<glm::vec2>()) args.emplace_back(obj.as<glm::vec2>());
                else if (obj.is<glm::vec3>()) args.emplace_back(obj.as<glm::vec3>());
                else if (obj.is<Node3d*>()) args.emplace_back(obj.as<Node3d*>());
                else std::cerr << "[Lua Signal Warning] Unsupported arg type for '" << sig << "'.\n";
            }
            src->QueueSignal(sig, args);
        }
    );
}