
#include "MeshNode3d.h"

#include "Core/RenderApi.h"

// for inspector use
MeshNode3d::MeshNode3d(Shader* shader) : m_shader(shader), m_material(std::make_shared<Material>()) {
    Init();
}

MeshNode3d::MeshNode3d(std::shared_ptr<Mesh> mesh, Shader* shader) : m_shader(shader), m_mesh(std::move(mesh)), m_material(std::make_shared<Material>()) {
    Init();
}

void MeshNode3d::SubmitToRenderer(RenderApi& renderer) {
    if (!m_mesh || !IsVisible()) return;
    renderer.SubmitMesh(this);
}

void MeshNode3d::Init() {
    SetName("MeshNode3d");
    AddProperty("visible",
        [this]() -> PropertyValue { return IsVisible(); },
        [this](const PropertyValue& v) { SetVisible(std::get<bool>(v)); }
    );
    AddProperty("material",
        [this]() -> PropertyValue { return std::static_pointer_cast<PropertyHolder>(m_material); },
        [this](const PropertyValue& v) {}
    );
}
