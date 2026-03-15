
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "tiny_gltf.h"
#include "GltfLoader.h"
#include "Mesh.h"
#include <iostream>

Node3d* GltfLoader::Load(const std::string& path, Shader* shader) {
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err, warn;

    const bool success = path.ends_with(".glb") ? loader.LoadBinaryFromFile(&model, &err, &warn, path) : loader.LoadASCIIFromFile(&model, &err, &warn, path);

    if (!warn.empty()) std::cerr << "GLTF Warning: " << warn << std::endl;
    if (!err.empty())  std::cerr << "GLTF Error: "   << err  << std::endl;
    if (!success) return nullptr;

    Node3d* root = new Node3d();

    for (auto& gltfMesh : model.meshes) {
        for (auto& primitive : gltfMesh.primitives) {
            // positions (required)
            if (!primitive.attributes.contains("POSITION")) continue;

            const auto& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
            const auto& posView = model.bufferViews[posAccessor.bufferView];
            const float* positions  = reinterpret_cast<const float*>(
                model.buffers[posView.buffer].data.data() + posView.byteOffset + posAccessor.byteOffset
            );

            // normals (optional)
            const float* normals = nullptr;
            if (primitive.attributes.contains("NORMAL")) {
                const auto& normAccessor = model.accessors[primitive.attributes.at("NORMAL")];
                const auto& normView = model.bufferViews[normAccessor.bufferView];
                normals = reinterpret_cast<const float*>(
                    model.buffers[normView.buffer].data.data() + normView.byteOffset + normAccessor.byteOffset
                );
            }

            // uvs (optional)
            const float* uvs = nullptr;
            if (primitive.attributes.contains("TEXCOORD_0")) {
                const auto& uvAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
                const auto& uvView = model.bufferViews[uvAccessor.bufferView];
                uvs = reinterpret_cast<const float*>(
                    model.buffers[uvView.buffer].data.data() + uvView.byteOffset + uvAccessor.byteOffset
                );
            }

            // build vertices
            std::vector<Vertex> vertices;
            vertices.reserve(posAccessor.count);
            for (size_t i = 0; i < posAccessor.count; i++) {
                Vertex v{};
                v.position = { positions[i*3], positions[i*3+1], positions[i*3+2] };
                v.normal = normals ? glm::vec3(normals[i*3], normals[i*3+1], normals[i*3+2]) : glm::vec3(0, 1, 0);
                v.texCoord = uvs ? glm::vec2(uvs[i*2], uvs[i*2+1]) : glm::vec2(0, 0);
                vertices.push_back(v);
            }

            // build indices
            std::vector<uint32_t> indices;
            if (primitive.indices >= 0) {
                const auto& idxAccessor = model.accessors[primitive.indices];
                const auto& idxView     = model.bufferViews[idxAccessor.bufferView];
                const uint8_t* idxData  = model.buffers[idxView.buffer].data.data() + idxView.byteOffset + idxAccessor.byteOffset;

                indices.reserve(idxAccessor.count);
                for (size_t i = 0; i < idxAccessor.count; i++) {
                    switch (idxAccessor.componentType) {
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
                            indices.push_back(idxData[i]);
                            break;
                        }
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                            indices.push_back(reinterpret_cast<const uint16_t*>(idxData)[i]);
                            break;
                        }
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                            indices.push_back(reinterpret_cast<const uint32_t*>(idxData)[i]);
                            break;
                        }
                        default: {
                            std::cerr << "GLTF: unsupported index component type" << std::endl;
                            break;
                        }
                    }
                }
            }

            // double sided material flag
            if (primitive.material >= 0) {
                const auto& mat = model.materials[primitive.material];
                if (mat.doubleSided) {
                    glDisable(GL_CULL_FACE);
                } else {
                    glEnable(GL_CULL_FACE);
                }
            }

            auto* mesh = new Mesh(vertices, indices);
            auto* node = new MeshNode3d(mesh, shader);
            root->AddChild(node);
        }
    }

    return root;
}
