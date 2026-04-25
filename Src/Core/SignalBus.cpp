
#include "SignalBus.h"

#include <algorithm>
#include <ranges>

#include "ScriptManager.h"
#include "Nodes/Node3d.h"

SignalBus& SignalBus::Get() {
    static SignalBus instance;
    return instance;
}

void SignalBus::Connect(Node3d* source, const std::string& signal, Node3d* target, const std::string& method, const bool oneShot) {
    if (!source || !target || signal.empty() || method.empty()) {
        return;
    }

    std::lock_guard lock(m_mutex);

    auto& connections = m_connections[source][signal];

    for (const auto& connection : connections) {
        if (connection.target == target && connection.method == method) {
            return;
        }
    }

    connections.push_back({
        .target = target,
        .method = method,
        .oneShot = oneShot
    });
}

void SignalBus::Disconnect(Node3d* source, const std::string& signal, Node3d* target, const std::string& method) {
    if (!source || !target) {
        return;
    }

    std::lock_guard lock(m_mutex);

    const auto sourceIt = m_connections.find(source);
    if (sourceIt == m_connections.end()) {
        return;
    }

    const auto signalIt = sourceIt->second.find(signal);
    if (signalIt == sourceIt->second.end()) {
        return;
    }

    auto& connections = signalIt->second;

    std::erase_if(connections, [&](const SignalConnection& connection) {
        return connection.target == target && connection.method == method;
    });

    if (connections.empty()) {
        sourceIt->second.erase(signalIt);
    }

    if (sourceIt->second.empty()) {
        m_connections.erase(sourceIt);
    }
}

void SignalBus::Emit(Node3d* source, const std::string& signal, const std::vector<SignalArg>& args) {
    if (!source || signal.empty()) {
        return;
    }

    std::vector<SignalConnection> connections;

    {
        std::lock_guard lock(m_mutex);

        const auto sourceIt = m_connections.find(source);
        if (sourceIt == m_connections.end()) {
            return;
        }

        const auto signalIt = sourceIt->second.find(signal);
        if (signalIt == sourceIt->second.end()) {
            return;
        }

        connections = signalIt->second;
    }

    std::vector<SignalConnection> oneShotConnections;

    for (const auto& connection : connections) {
        if (!connection.target) {
            continue;
        }

        ScriptManager::Get().CallSignalMethod(
            connection.target,
            connection.method,
            args
        );

        if (connection.oneShot) {
            oneShotConnections.push_back(connection);
        }
    }

    for (const auto& connection : oneShotConnections) {
        Disconnect(source, signal, connection.target, connection.method);
    }
}

void SignalBus::Queue(Node3d* source, const std::string& signal, const std::vector<SignalArg>& args) {
    if (!source || signal.empty()) {
        return;
    }

    std::lock_guard lock(m_mutex);

    m_queue.push_back({
        .source = source,
        .name = signal,
        .args = args
    });
}

void SignalBus::DispatchQueued() {
    std::vector<QueuedSignal> queue;

    {
        std::lock_guard lock(m_mutex);
        queue.swap(m_queue);
    }

    for (const QueuedSignal& event : queue) {
        Emit(event.source, event.name, event.args);
    }
}

void SignalBus::RemoveNode(Node3d* node) {
    if (!node) {
        return;
    }

    std::lock_guard lock(m_mutex);

    m_connections.erase(node);

    for (auto &signalMap: m_connections | std::views::values) {
        for (auto &connections: signalMap | std::views::values) {
            std::erase_if(connections, [&](const SignalConnection& connection) {
                return connection.target == node;
            });
        }

        std::erase_if(signalMap, [](const auto& pair) {
            return pair.second.empty();
        });
    }

    std::erase_if(m_connections, [](const auto& pair) {
        return pair.second.empty();
    });

    std::erase_if(m_queue, [&](const QueuedSignal& event) {
        if (event.source == node) {
            return true;
        }

        for (const SignalArg& arg : event.args) {
            if (std::holds_alternative<Node3d*>(arg) && std::get<Node3d*>(arg) == node) {
                return true;
            }
        }

        return false;
    });
}