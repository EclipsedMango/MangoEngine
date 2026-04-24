#ifndef MANGORENDERING_MESH_H
#define MANGORENDERING_MESH_H

#include <memory>
#include <vector>
#include <cstddef>
#include <cstdint>

#include "Renderer/VertexArray.h"
#include "Renderer/Buffers/ShaderStorageBuffer.h"
#include "Core/PropertyHolder.h"

class Mesh : public PropertyHolder {
public:
    // ONLY for internal use and serializing, use other constructors instead
    Mesh();
    Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    ~Mesh() override = default;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    void Upload();
    void Regenerate(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    [[nodiscard]] std::string GetPropertyHolderType() const override { return "Mesh"; }

    [[nodiscard]] bool IsUploaded() const { return m_buffer != nullptr; }
    [[nodiscard]] uint64_t GetResourceId() const { return m_resourceId; }

    [[nodiscard]] const std::vector<Vertex>& GetVertices() const { return m_vertices; }
    [[nodiscard]] const std::vector<uint32_t>& GetIndices() const { return m_indices; }
    [[nodiscard]] size_t GetVertexCount() const { return m_vertices.size(); }
    [[nodiscard]] VertexArray* GetBuffer() const { return m_buffer.get(); }
    [[nodiscard]] bool HasSkinWeights() const { return m_hasSkinWeights; }
    [[nodiscard]] ShaderStorageBuffer* GetSkinningSourceBuffer() const;

    [[nodiscard]] glm::vec3 GetBoundsCenter() const { return m_boundsCenter; }
    [[nodiscard]] float GetBoundsRadius() const { return m_boundsRadius; }

    void SetSourcePath(const std::string& path) { m_sourcePath = path; }
    [[nodiscard]] const std::string& GetSourcePath() const { return m_sourcePath; }

private:
    void RegisterProperties();
    static uint64_t GenerateResourceId();
    void ComputeBounds();
    void ComputeSkinWeightsUsage();
    void EnsureSkinningSourceBuffer() const;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    std::unique_ptr<VertexArray> m_buffer;
    uint64_t m_resourceId = 0;

    glm::vec3 m_boundsCenter = glm::vec3(0.0f);
    float m_boundsRadius = 0.0f;
    bool m_hasSkinWeights = false;
    mutable std::unique_ptr<ShaderStorageBuffer> m_skinningSourceSsbo;
    mutable bool m_skinningSourceUploaded = false;

    std::string m_sourcePath;
};


#endif //MANGORENDERING_MESH_H
