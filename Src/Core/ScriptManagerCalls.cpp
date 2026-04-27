
#include "ScriptManager.h"
#include "ScriptManagerInternal.h"

#include <tracy/Tracy.hpp>
#include <iostream>

#include "Nodes/Node3d.h"

void ScriptManager::CallReady(const Node3d* node) const {
    ZoneScoped;
    if (!m_impl->runtimeEnabled) return;

    const auto it = m_impl->scripts.find(const_cast<Node3d*>(node));
    if (it == m_impl->scripts.end()) return;

    auto& instance = it->second;

    if (!instance.initialized && instance.init.valid()) {
        const auto initResult = instance.init(instance.table);
        ReportProtectedFunctionError(initResult, "_init");
        if (!initResult.valid()) return;
        instance.initialized = true;
    }

    if (!instance.ready.valid()) return;

    const auto result = instance.ready(instance.table);
    ReportProtectedFunctionError(result, "_ready");
}

void ScriptManager::CallProcess(Node3d* node, float deltaTime) const {
    ZoneScoped;
    if (!m_impl->runtimeEnabled) return;

    const auto it = m_impl->scripts.find(node);
    if (it == m_impl->scripts.end() || !it->second.process.valid()) return;

    const auto result = it->second.process(it->second.table, deltaTime);
    ReportProtectedFunctionError(result, "_process");
}

void ScriptManager::CallPhysicsProcess(Node3d* node, float deltaTime) const {
    ZoneScoped;
    if (!m_impl->runtimeEnabled) return;

    const auto it = m_impl->scripts.find(node);
    if (it == m_impl->scripts.end() || !it->second.physicsProcess.valid()) return;

    const auto result = it->second.physicsProcess(it->second.table, deltaTime);
    ReportProtectedFunctionError(result, "_physics_process");
}

void ScriptManager::CallSignalMethod(Node3d* target, const std::string& methodName, const std::vector<SignalArg>& args) const {
    ZoneScoped;
    if (!m_impl->runtimeEnabled || !target || methodName.empty()) return;

    const auto it = m_impl->scripts.find(target);
    if (it == m_impl->scripts.end()) return;

    const sol::object methodObject = it->second.table[methodName];
    if (!methodObject.valid() || methodObject == sol::lua_nil) return;

    if (!methodObject.is<sol::protected_function>()) {
        std::cerr << "[Signal Warning] Method '" << methodName << "' exists but is not callable on node '" << target->GetName() << "'.\n";
        return;
    }

    const sol::protected_function method = methodObject.as<sol::protected_function>();

    std::vector<sol::object> luaArgs;
    luaArgs.reserve(args.size());

    const sol::state_view luaView(m_impl->lua);
    for (const SignalArg& arg : args) {
        luaArgs.push_back(SignalArgToLua(arg, luaView));
    }

    const auto result = method(it->second.table, sol::as_args(luaArgs));
    ReportProtectedFunctionError(result, "signal method " + methodName);
}