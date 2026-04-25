
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

void SignalBus::ConnectNative(Node3d *source, const std::string &signal, Node3d *owner, NativeSignalCallback callback, bool oneShot) {
    if (!source || !owner || signal.empty() || !callback) {
        return;
    }

    std::lock_guard lock(m_mutex);
    auto& connections = m_nativeConnections[source][signal];

    for (const auto& connection : connections) {
        if (connection.owner == owner) {
            return;
        }
    }

    connections.push_back({
        .owner = owner,
        .callback = std::move(callback),
        .oneShot = oneShot
    });
}

void SignalBus::DisconnectNative(Node3d *source, const std::string &signal, Node3d *owner) {
    if (!source || !owner) {
        return;
    }

    std::lock_guard lock(m_mutex);

    const auto sourceIt = m_nativeConnections.find(source);
    if (sourceIt == m_nativeConnections.end()) {
        return;
    }

    const auto signalIt = sourceIt->second.find(signal);
    if (signalIt == sourceIt->second.end()) {
        return;
    }

    auto& connections = signalIt->second;

    std::erase_if(connections, [&](const NativeSignalConnection& connection) {
        return connection.owner == owner;
    });

    if (connections.empty()) {
        sourceIt->second.erase(signalIt);
    }

    if (sourceIt->second.empty()) {
        m_nativeConnections.erase(sourceIt);
    }
}

void SignalBus::Emit(Node3d* source, const std::string& signal, const std::vector<SignalArg>& args) {
    if (!source || signal.empty()) {
        return;
    }

    std::vector<SignalConnection> scriptConnections;
    std::vector<NativeSignalConnection> nativeConnections;

    {
        std::lock_guard lock(m_mutex);

        if (const auto sourceIt = m_connections.find(source); sourceIt != m_connections.end()) {
            if (const auto signalIt = sourceIt->second.find(signal); signalIt != sourceIt->second.end()) {
                scriptConnections = signalIt->second;
            }
        }

        if (const auto sourceIt = m_nativeConnections.find(source); sourceIt != m_nativeConnections.end()) {
            if (const auto signalIt = sourceIt->second.find(signal); signalIt != sourceIt->second.end()) {
                nativeConnections = signalIt->second;
            }
        }
    }

    std::vector<SignalConnection> oneShotScriptConnections;
    std::vector<NativeSignalConnection> oneShotNativeConnections;

    for (const auto& connection : scriptConnections) {
        if (!connection.target) {
            continue;
        }

        ScriptManager::Get().CallSignalMethod(
            connection.target,
            connection.method,
            args
        );

        if (connection.oneShot) {
            oneShotScriptConnections.push_back(connection);
        }
    }

    for (const auto& connection : nativeConnections) {
        if (!connection.owner || !connection.callback) {
            continue;
        }

        connection.callback(args);

        if (connection.oneShot) {
            oneShotNativeConnections.push_back(connection);
        }
    }

    for (const auto& connection : oneShotScriptConnections) {
        Disconnect(source, signal, connection.target, connection.method);
    }

    for (const auto& connection : oneShotNativeConnections) {
        DisconnectNative(source, signal, connection.owner);
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
    m_nativeConnections.erase(node);

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

    for (auto& signalMap : m_nativeConnections | std::views::values) {
        for (auto& connections : signalMap | std::views::values) {
            std::erase_if(connections, [&](const NativeSignalConnection& connection) {
                return connection.owner == node;
            });
        }

        std::erase_if(signalMap, [](const auto& pair) {
            return pair.second.empty();
        });
    }

    std::erase_if(m_nativeConnections, [](const auto& pair) {
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