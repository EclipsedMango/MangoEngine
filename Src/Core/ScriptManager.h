
#ifndef MANGORENDERING_SCRIPTMANAGER_H
#define MANGORENDERING_SCRIPTMANAGER_H

#include <functional>
#include <memory>
#include <string>

#include "SignalBus.h"

class Node3d;

class ScriptManager {
public:
    static ScriptManager& Get() {
        static ScriptManager instance;
        return instance;
    }

    void Init();

    void SetScript(Node3d* node, const std::string& path) const;
    void ClearScript(Node3d* node) const;
    void SetRuntimeEnabled(bool enabled) const;
    void SetQuitHandler(std::function<void()> handler) const;
    void RequestQuit() const;
    [[nodiscard]] bool IsRuntimeEnabled() const;

    void CallReady(const Node3d* node) const;
    void CallProcess(Node3d* node, float deltaTime) const;
    void CallPhysicsProcess(Node3d* node, float deltaTime) const;

    void CallSignalMethod(Node3d* target, const std::string& methodName, const std::vector<SignalArg>& args) const;

    ScriptManager(const ScriptManager&) = delete;
    ScriptManager& operator=(const ScriptManager&) = delete;

private:
    ScriptManager();
    ~ScriptManager();

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};


#endif //MANGORENDERING_SCRIPTMANAGER_H
