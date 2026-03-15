
#ifndef MANGORENDERING_TREELISTENER_H
#define MANGORENDERING_TREELISTENER_H

class Node3d;

enum class NodeNotification {
    EnterTree,
    ExitTree,
    Ready,
};

class TreeListener {
public:
    virtual ~TreeListener() = default;
    virtual void Notification(Node3d* node, NodeNotification notification) = 0;
};


#endif //MANGORENDERING_TREELISTENER_H