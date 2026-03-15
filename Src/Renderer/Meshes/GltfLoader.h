#ifndef MANGORENDERING_GLTFLOADER_H
#define MANGORENDERING_GLTFLOADER_H

#include <string>
#include <vector>

#include "Nodes/MeshNode3d.h"
#include "Renderer/Shader.h"

class GltfLoader {
public:
    // returns a root Node3d with all meshes as children, caller owns the pointer
    static Node3d* Load(const std::string& path, Shader* shader);
};

#endif //MANGORENDERING_GLTFLOADER_H