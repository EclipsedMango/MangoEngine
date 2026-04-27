
#include "ScriptManager.h"
#include "ScriptManagerInternal.h"

#include <glm/glm.hpp>
#include <iostream>

void RegisterMathTypes(sol::state& lua) {
    lua.new_usertype<glm::vec2>("vec2",
        sol::constructors<glm::vec2(), glm::vec2(float), glm::vec2(float, float)>(),
        "x", &glm::vec2::x,
        "y", &glm::vec2::y,
        sol::meta_function::addition, [](const glm::vec2& a, const glm::vec2& b) { return a + b; },
        sol::meta_function::subtraction, [](const glm::vec2& a, const glm::vec2& b) { return a - b; },
        sol::meta_function::multiplication, [](const glm::vec2& a, const float s) { return a * s; },
        sol::meta_function::division, [](const glm::vec2& a, const float s) { return a / s; }
    );

    lua.new_usertype<glm::vec3>("vec3",
        sol::constructors<glm::vec3(), glm::vec3(float), glm::vec3(float, float, float)>(),
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z,
        sol::meta_function::addition, [](const glm::vec3& a, const glm::vec3& b) { return a + b; },
        sol::meta_function::subtraction, [](const glm::vec3& a, const glm::vec3& b) { return a - b; },
        sol::meta_function::multiplication, [](const glm::vec3& a, const float s) { return a * s; },
        sol::meta_function::division, [](const glm::vec3& a, const float s) { return a / s; }
    );
}

void RegisterPropertyHolder(sol::state& lua) {
    lua.new_usertype<PropertyHolder>("PropertyHolder",
        "Get", [](const PropertyHolder* holder, const std::string& name, const sol::this_state s) -> sol::object {
            const sol::state_view lua2(s);
            try {
                return PropertyValueToLua(holder->GetProperty(name), lua2);
            } catch (const std::exception& e) {
                std::cerr << "[Lua Warning] Failed to get nested property '" << name << "': " << e.what() << "\n";
                return sol::make_object(lua2, sol::lua_nil);
            }
        },
        "Set", [](PropertyHolder* holder, const std::string& name, const sol::object& val) {
            try {
                holder->Set(name, LuaToPropertyValue(val));
            } catch (const std::exception& e) {
                std::cerr << "[Lua Warning] Failed to set nested property '" << name << "': " << e.what() << "\n";
            }
        }
    );
}