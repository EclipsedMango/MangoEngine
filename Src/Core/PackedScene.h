
#ifndef MANGORENDERING_PACKEDSCENE_H
#define MANGORENDERING_PACKEDSCENE_H

class Node3d;

class PackedScene {
public:
    PackedScene(Node3d* node);

    Node3d* Instantiate();

private:
    Node3d* m_node = nullptr;
};


#endif //MANGORENDERING_PACKEDSCENE_H