
#ifndef MANGORENDERING_SIGNALBUS_H
#define MANGORENDERING_SIGNALBUS_H

#include <mutex>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

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

class SignalBus {
public:
    static SignalBus& Get();

    void Connect(Node3d* source, const std::string& signal, Node3d* target, const std::string& method, bool oneShot = false);
    void Disconnect(Node3d* source, const std::string& signal, Node3d* target, const std::string& method);
    void Emit(Node3d* source, const std::string& signal, const std::vector<SignalArg>& args = {});
    void Queue(Node3d* source, const std::string& signal, const std::vector<SignalArg>& args = {});

    void DispatchQueued();
    void RemoveNode(Node3d* node);

private:
    std::mutex m_mutex;

    std::unordered_map<Node3d*, std::unordered_map<std::string, std::vector<SignalConnection>>> m_connections;
    std::vector<QueuedSignal> m_queue;
};



#endif //MANGORENDERING_SIGNALBUS_H
