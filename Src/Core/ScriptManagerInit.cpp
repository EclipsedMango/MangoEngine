
#include "ScriptManager.h"
#include "ScriptManagerInternal.h"

#include <iostream>

#include "Nodes/RigidBody3d.h"

void ScriptManager::Init() {
    auto& lua = m_impl->lua;

    lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math, sol::lib::string, sol::lib::table);

    lua.set_panic([](lua_State* L) -> int {
        const char* msg = lua_tostring(L, -1);
        std::cerr << "[Lua Panic Error] " << (msg ? msg : "Unknown Error") << std::endl;
        return 0;
    });

    RegisterMathTypes(lua);
    RegisterPropertyHolder(lua);
    RegisterNode3d(lua);
    RegisterDerivedNodes(lua);
    RegisterInputModule(lua);
    RegisterAppModule(lua, this);
}