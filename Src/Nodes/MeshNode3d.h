
#ifndef MANGORENDERING_MESHNODE3D_H
#define MANGORENDERING_MESHNODE3D_H

#include "RenderableNode3d.h"
#include "Renderer/Materials/Material.h"
#include "Renderer/Meshes/Mesh.h"

class MeshNode3d : public RenderableNode3d {
public:
    MeshNode3d();
    explicit MeshNode3d(std::shared_ptr<Mesh> mesh);

    [[nodiscard]] std::unique_ptr<Node3d> Clone() override;

    void SetMesh(std::shared_ptr<Mesh> mesh) { m_mesh = std::move(mesh); }
    void SetMeshByName(const std::string& name);
    [[nodiscard]] Mesh* GetMesh() const { return m_mesh ? m_mesh.get() : nullptr; }
    [[nodiscard]] std::shared_ptr<Mesh> GetMeshPtr() const { return m_mesh; }

    void SetMaterial(const std::shared_ptr<Material> &material) { m_material = material; }
    void SetMaterialOverride(const std::shared_ptr<Material> &material) { m_materialOverride = material; }
    void ClearMaterialOverride() { m_materialOverride = nullptr; }

    [[nodiscard]] Material* GetActiveMaterial() { return m_materialOverride ? m_materialOverride.get() : m_material.get(); }
    [[nodiscard]] const Material* GetActiveMaterial() const { return m_materialOverride ? m_materialOverride.get() : m_material.get(); }
    [[nodiscard]] std::shared_ptr<Material> GetMaterialPtr() { return m_material; }

    [[nodiscard]] std::string GetNodeType() const override { return "MeshNode3d"; }

private:
    virtual void Init();

    std::shared_ptr<PropertyHolder> m_meshSlot;

    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<Material> m_material;
    std::shared_ptr<Material> m_materialOverride;
};


#endif //MANGORENDERING_MESHNODE3D_H