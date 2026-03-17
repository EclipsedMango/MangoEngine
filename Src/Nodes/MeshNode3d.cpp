
#include "MeshNode3d.h"
#include <stdexcept>

#include "Core/RenderApi.h"

// for inspector use
MeshNode3d::MeshNode3d(Shader* shader) : m_shader(shader), m_material(std::make_shared<Material>()) {
    SetName("MeshNode3d");
}

MeshNode3d::MeshNode3d(std::shared_ptr<Mesh> mesh, Shader* shader) : m_shader(shader), m_mesh(std::move(mesh)), m_material(std::make_shared<Material>()) {
    SetName("MeshNode3d");
}

void MeshNode3d::SubmitToRenderer(RenderApi& renderer) {
    if (!m_mesh || !IsVisible()) return;
    renderer.SubmitMesh(this);
}
