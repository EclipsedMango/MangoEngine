
#include "ScriptManager.h"
#include "Nodes/Node3d.h"
#include "ScriptManagerInternal.h"

#include <glm/glm.hpp>
#include <iostream>
#include <variant>

void ReportProtectedFunctionError(const sol::protected_function_result& result, const std::string& context) {
    if (result.valid()) return;
    const sol::error err = result;
    std::cerr << "[Lua Error] " << context << ":\n" << err.what() << std::endl;
}

sol::object SignalArgToLua(const SignalArg& arg, const sol::state_view& lua) {
    if (std::holds_alternative<int>(arg))
        return sol::make_object(lua, std::get<int>(arg));
    if (std::holds_alternative<float>(arg))
        return sol::make_object(lua, std::get<float>(arg));
    if (std::holds_alternative<bool>(arg))
        return sol::make_object(lua, std::get<bool>(arg));
    if (std::holds_alternative<std::string>(arg))
        return sol::make_object(lua, std::get<std::string>(arg));
    if (std::holds_alternative<glm::vec2>(arg))
        return sol::make_object(lua, std::get<glm::vec2>(arg));
    if (std::holds_alternative<glm::vec3>(arg))
        return sol::make_object(lua, std::get<glm::vec3>(arg));
    if (std::holds_alternative<Node3d*>(arg)) {
        Node3d* node = std::get<Node3d*>(arg);
        return node ? sol::make_object(lua, node) : sol::lua_nil;
    }
    return sol::lua_nil;
}

sol::object PropertyValueToLua(const PropertyValue& val, const sol::state_view& lua) {
    if (std::holds_alternative<int>(val)) return sol::make_object(lua, std::get<int>(val));
    if (std::holds_alternative<float>(val)) return sol::make_object(lua, std::get<float>(val));
    if (std::holds_alternative<bool>(val)) return sol::make_object(lua, std::get<bool>(val));
    if (std::holds_alternative<std::string>(val)) return sol::make_object(lua, std::get<std::string>(val));
    if (std::holds_alternative<glm::vec2>(val)) return sol::make_object(lua, std::get<glm::vec2>(val));
    if (std::holds_alternative<glm::vec3>(val)) return sol::make_object(lua, std::get<glm::vec3>(val));
    if (std::holds_alternative<std::shared_ptr<PropertyHolder>>(val)) {
        auto ptr = std::get<std::shared_ptr<PropertyHolder>>(val);
        return ptr ? sol::make_object(lua, ptr) : sol::lua_nil;
    }
    return sol::make_object(lua, sol::lua_nil);
}

PropertyValue LuaToPropertyValue(const sol::object& luaVal) {
    if (luaVal.is<bool>()) return luaVal.as<bool>();
    if (luaVal.is<std::string>()) return luaVal.as<std::string>();
    if (luaVal.is<glm::vec2>()) return luaVal.as<glm::vec2>();
    if (luaVal.is<glm::vec3>()) return luaVal.as<glm::vec3>();
    if (luaVal.is<std::shared_ptr<PropertyHolder>>())
        return luaVal.as<std::shared_ptr<PropertyHolder>>();

    if (luaVal.is<double>()) {
        const double v = luaVal.as<double>();
        return static_cast<float>(v);
    }

    throw std::runtime_error("Unsupported Lua type passed to PropertyValue");
}

ScriptManager::ScriptManager() : m_impl(std::make_unique<Impl>()) {}
ScriptManager::~ScriptManager() = default;

void ScriptManager::SetRuntimeEnabled(const bool enabled) const {
    m_impl->runtimeEnabled = enabled;
}

bool ScriptManager::IsRuntimeEnabled() const {
    return m_impl->runtimeEnabled;
}

void ScriptManager::SetQuitHandler(std::function<void()> handler) const {
    m_impl->quitHandler = std::move(handler);
}

void ScriptManager::RequestQuit() const {
    if (!m_impl->quitHandler) {
        std::cerr << "[Script Warning] Quit requested, but no quit handler is registered.\n";
        return;
    }
    m_impl->quitHandler();
}