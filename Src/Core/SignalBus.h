
#ifndef MANGORENDERING_SIGNALBUS_H
#define MANGORENDERING_SIGNALBUS_H

#include <mutex>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <functional>

#include <glm/glm.hpp>

class Node3d;

using SignalArg = std::variant<
    int,
    float,
    bool,
    std::string,
    glm::vec2,
    glm::vec3,
    Node3d*
>;

struct SignalConnection {
    Node3d* target = nullptr;
    std::string method;
    bool oneShot = false;
};

struct QueuedSignal {
    Node3d* source = nullptr;
    std::string name;
    std::vector<SignalArg> args;
};

using NativeSignalCallback = std::function<void(const std::vector<SignalArg>&)>;

struct NativeSignalConnection {
    Node3d* owner = nullptr;
    NativeSignalCallback callback;
    bool oneShot = false;
};

class SignalBus {
public:
    static SignalBus& Get();

    void Connect(Node3d* source, const std::string& signal, Node3d* target, const std::string& method, bool oneShot = false);
    void Disconnect(Node3d* source, const std::string& signal, Node3d* target, const std::string& method);

    void ConnectNative(Node3d* source, const std::string& signal, Node3d* owner, NativeSignalCallback callback, bool oneShot = false);
    void DisconnectNative(Node3d* source, const std::string& signal, Node3d* owner);

    void Emit(Node3d* source, const std::string& signal, const std::vector<SignalArg>& args = {});
    void Queue(Node3d* source, const std::string& signal, const std::vector<SignalArg>& args = {});

    void DispatchQueued();
    void RemoveNode(Node3d* node);

private:
    std::mutex m_mutex;

    // connections are for lua
    std::unordered_map<Node3d*, std::unordered_map<std::string, std::vector<SignalConnection>>> m_connections;
    std::unordered_map<Node3d*, std::unordered_map<std::string, std::vector<NativeSignalConnection>>> m_nativeConnections;
    std::vector<QueuedSignal> m_queue;
};



#endif //MANGORENDERING_SIGNALBUS_H
