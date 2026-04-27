
#include "ScriptManager.h"
#include "ScriptManagerInternal.h"

#include <tracy/Tracy.hpp>
#include <iostream>
#include <ranges>
#include <unordered_set>

#include "ResourceManager.h"
#include "Nodes/Node3d.h"

void ScriptManager::SetScript(Node3d* node, const std::string& path) const {
    ZoneScoped;
    ClearScript(node);

    if (!node || path.empty()) return;

    auto& lua = m_impl->lua;

    const std::string resolvedPath = ResourceManager::Get().ResolveAssetPath(path);
    if (resolvedPath.empty()) {
        std::cerr << "[Script Error] Failed to resolve script asset: " << path << std::endl;
        return;
    }

    try {
        m_impl->scriptModTimes[resolvedPath] = std::filesystem::last_write_time(resolvedPath);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "[Script Warning] Failed to cache mod time for " << resolvedPath << ": " << e.what() << "\n";
    }

    std::cout << "[Script] Loading from disk: " << resolvedPath << std::endl;

    sol::load_result loaded = lua.load_file(resolvedPath);
    if (!loaded.valid()) {
        const sol::error err = loaded;
        std::cerr << "[Script Error] Failed to load " << path << " (" << resolvedPath << "):\n" << err.what() << std::endl;
        return;
    }

    const sol::protected_function_result execResult = loaded();
    if (!execResult.valid()) {
        ReportProtectedFunctionError(execResult, "Failed to execute script file " + path + " (" + resolvedPath + ")");
        return;
    }

    const sol::object returned = execResult.get<sol::object>();
    if (!returned.is<sol::table>()) {
        std::cerr << "[Script Error] Script " << path << " must return a table." << std::endl;
        return;
    }

    sol::table scriptClass = returned.as<sol::table>();

    Impl::ScriptInstance instance;
    instance.table = lua.create_table();

    sol::table metatable = lua.create_table();
    metatable["__index"] = scriptClass;
    instance.table[sol::metatable_key] = metatable;

    instance.table["node"] = node;
    instance.table["script_path"] = resolvedPath;
    instance.table["script_name"] = path;

    instance.init = scriptClass["_init"];
    instance.ready = scriptClass["_ready"];
    instance.process = scriptClass["_process"];
    instance.physicsProcess = scriptClass["_physics_process"];

    m_impl->scripts[node] = std::move(instance);
}

void ScriptManager::ClearScript(Node3d* node) const {
    if (node) m_impl->scripts.erase(node);
}

void ScriptManager::HotReloadScript(Node3d* node) const {
    const auto it = m_impl->scripts.find(node);
    if (it == m_impl->scripts.end()) return;

    const std::string scriptName = it->second.table.get<std::string>("script_name");
    SetScript(node, scriptName);
}

void ScriptManager::PollHotReload() const {
    std::unordered_set<std::string> changedPaths;

    for (auto& instance : m_impl->scripts | std::views::values) {
        const std::string path = instance.table["script_path"];
        if (path.empty()) continue;

        try {
            const auto currentTime = std::filesystem::last_write_time(path);
            auto [it, inserted] = m_impl->scriptModTimes.try_emplace(path, currentTime);
            if (!inserted && currentTime != it->second) {
                it->second = currentTime;
                changedPaths.insert(path);
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "[HotReload] Failed to stat file: " << path << " - " << e.what() << "\n";
        }
    }

    if (changedPaths.empty()) return;

    std::vector<Node3d*> nodesToReload;
    for (auto& [node, instance] : m_impl->scripts) {
        const std::string path = instance.table["script_path"];
        if (changedPaths.contains(path)) {
            nodesToReload.push_back(node);
        }
    }

    for (Node3d* node : nodesToReload) {
        const auto it = m_impl->scripts.find(node);
        if (it == m_impl->scripts.end()) continue;

        const std::string path = it->second.table["script_path"];
        std::cout << "[HotReload] Reloading: " << path << std::endl;

        HotReloadScript(node);
        CallReady(node);
    }
}