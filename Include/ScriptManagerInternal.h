
#ifndef MANGORENDERING_SCRIPTMANAGERINTERNAL_H
#define MANGORENDERING_SCRIPTMANAGERINTERNAL_H

#include <sol/sol.hpp>
#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>

#include "Core/PropertyHolder.h"
#include "Core/ScriptManager.h"
#include "Core/SignalBus.h"

class Node3d;

struct ScriptManager::Impl {
    struct ScriptInstance {
        sol::table table;
        sol::protected_function init;
        sol::protected_function ready;
        sol::protected_function process;
        sol::protected_function physicsProcess;
        bool initialized = false;
    };

    sol::state lua;
    std::unordered_map<Node3d*, ScriptInstance> scripts;
    std::unordered_map<std::string, std::filesystem::file_time_type> scriptModTimes;
    bool runtimeEnabled = true;
    std::function<void()> quitHandler;
};


sol::object SignalArgToLua(const SignalArg& arg, const sol::state_view& lua);
sol::object PropertyValueToLua(const PropertyValue& val, const sol::state_view& lua);
PropertyValue LuaToPropertyValue(const sol::object& luaVal);
void ReportProtectedFunctionError(const sol::protected_function_result& result, const std::string& context);

// ScriptManagerInitMath.cpp
void RegisterMathTypes(sol::state& lua);
void RegisterPropertyHolder(sol::state& lua);

// ScriptManagerInitNode.cpp
void RegisterNode3d(sol::state& lua);

// ScriptManagerInitNodes.cpp  (derived node types)
void RegisterDerivedNodes(sol::state& lua);

// ScriptManagerInitModules.cpp
void RegisterInputModule(sol::state& lua);
void RegisterAppModule(sol::state& lua, ScriptManager* mgr);

#endif //MANGORENDERING_SCRIPTMANAGERINTERNAL_H
