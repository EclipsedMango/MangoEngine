#ifndef MANGORENDERING_GLTFLOADER_H
#define MANGORENDERING_GLTFLOADER_H

#include <string>
#include "tiny_gltf.h"
#include "Nodes/MeshNode3d.h"
#include "Renderer/Shader.h"

class GltfLoader {
public:
    // loads the full scene hierarchy, caller owns the returned tree
    static std::unique_ptr<Node3d> Load(const std::string& path, std::shared_ptr<Shader> shader);

    // extracts a specific sub mesh
    static std::shared_ptr<Mesh> ExtractMesh(const std::string& path, int meshIndex, int primIndex);

private:
    static std::shared_ptr<tinygltf::Model> GetParsedModel(const std::string& path);
};

#endif //MANGORENDERING_GLTFLOADER_H