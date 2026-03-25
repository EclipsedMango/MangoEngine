// PrimitiveMesh.cpp
#include "PrimitiveMesh.h"
#include <cmath>

void PrimitiveMesh::Rebuild() {
    BuildGeometry();
}

// ----- cube mesh -----
CubeMesh::CubeMesh(const float size) : m_size(size) {
    CubeMesh::BuildGeometry();
}

void CubeMesh::BuildGeometry() {
    const float h = m_size * 0.5f;

    const std::vector<Vertex> vertices = {
        // back
        {{-h,-h,-h},{0,0,-1},{0,0}}, {{ h,-h,-h},{0,0,-1},{1,0}},
        {{ h, h,-h},{0,0,-1},{1,1}}, {{-h, h,-h},{0,0,-1},{0,1}},
        // front
        {{-h,-h, h},{0,0,1},{0,0}},  {{ h,-h, h},{0,0,1},{1,0}},
        {{ h, h, h},{0,0,1},{1,1}},  {{-h, h, h},{0,0,1},{0,1}},
        // left
        {{-h,-h,-h},{-1,0,0},{1,0}}, {{-h,-h, h},{-1,0,0},{0,0}},
        {{-h, h, h},{-1,0,0},{0,1}}, {{-h, h,-h},{-1,0,0},{1,1}},
        // right
        {{ h,-h,-h},{1,0,0},{0,0}},  {{ h,-h, h},{1,0,0},{1,0}},
        {{ h, h, h},{1,0,0},{1,1}},  {{ h, h,-h},{1,0,0},{0,1}},
        // bottom
        {{-h,-h,-h},{0,-1,0},{0,1}}, {{ h,-h,-h},{0,-1,0},{1,1}},
        {{ h,-h, h},{0,-1,0},{1,0}}, {{-h,-h, h},{0,-1,0},{0,0}},
        // top
        {{-h, h,-h},{0,1,0},{0,1}},  {{ h, h,-h},{0,1,0},{1,1}},
        {{ h, h, h},{0,1,0},{1,0}},  {{-h, h, h},{0,1,0},{0,0}},
    };

    const std::vector<uint32_t> indices = {
        1,  0,  2,  2,  0,  3,
        4,  5,  6,  6,  7,  4,
        8,  9,  10, 10, 11, 8,
        13, 12, 14, 14, 12, 15,
        16, 17, 18, 18, 19, 16,
        21, 20, 22, 22, 20, 23,
    };

    Regenerate(vertices, indices);
}

// ----- plane mesh -----
PlaneMesh::PlaneMesh(const float width, const float depth, const int subdivisionsX, const int subdivisionsZ) : m_width(width), m_depth(depth), m_subdivisionsX(subdivisionsX), m_subdivisionsZ(subdivisionsZ) {
    PlaneMesh::BuildGeometry();
}

void PlaneMesh::BuildGeometry() {
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;

    const int vertsX = m_subdivisionsX + 1;
    const int vertsZ = m_subdivisionsZ + 1;

    for (int z = 0; z < vertsZ; z++) {
        for (int x = 0; x < vertsX; x++) {
            const float u = static_cast<float>(x) / m_subdivisionsX;
            const float v = static_cast<float>(z) / m_subdivisionsZ;
            vertices.push_back({
                { (u - 0.5f) * m_width, 0.0f, (v - 0.5f) * m_depth },
                { 0.0f, 1.0f, 0.0f },
                { u, v }
            });
        }
    }

    for (int z = 0; z < m_subdivisionsZ; z++) {
        for (int x = 0; x < m_subdivisionsX; x++) {
            const uint32_t tl = z * vertsX + x;
            const uint32_t tr = tl + 1;
            const uint32_t bl = tl + vertsX;
            const uint32_t br = bl + 1;
            indices.insert(indices.end(), { tl, bl, tr, tr, bl, br });
        }
    }

    Regenerate(vertices, indices);
}

// ----- quad mesh -----
QuadMesh::QuadMesh(const float width, const float height) : m_width(width), m_height(height) {
    QuadMesh::BuildGeometry();
}

void QuadMesh::BuildGeometry() {
    const float hw = m_width  * 0.5f;
    const float hh = m_height * 0.5f;

    const std::vector<Vertex> vertices = {
        {{-hw, -hh, 0}, {0, 0, 1}, {0, 0}, {1.0, 0.0, 0.0, 1.0}},
        {{ hw, -hh, 0}, {0, 0, 1}, {1, 0}, {1.0, 0.0, 0.0, 1.0}},
        {{ hw,  hh, 0}, {0, 0, 1}, {1, 1}, {1.0, 0.0, 0.0, 1.0}},
        {{-hw,  hh, 0}, {0, 0, 1}, {0, 1}, {1.0, 0.0, 0.0, 1.0}},
    };

    const std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

    Regenerate(vertices, indices);
}

// ----- sphere mesh -----
SphereMesh::SphereMesh(const float radius, const int rings, const int sectors) : m_radius(radius), m_rings(rings), m_sectors(sectors) {
    SphereMesh::BuildGeometry();
}

void SphereMesh::BuildGeometry() {
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;

    for (int r = 0; r <= m_rings; r++) {
        const float phi = PRIM_PI * static_cast<float>(r) / m_rings;
        for (int s = 0; s <= m_sectors; s++) {
            const float theta = 2.0f * PRIM_PI * static_cast<float>(s) / m_sectors;
            const float x = std::sin(phi) * std::cos(theta);
            const float y = std::cos(phi);
            const float z = std::sin(phi) * std::sin(theta);
            vertices.push_back({
                { x * m_radius, y * m_radius, z * m_radius },
                { x, y, z },
                { static_cast<float>(s) / m_sectors, static_cast<float>(r) / m_rings }
            });
        }
    }

    for (int r = 0; r < m_rings; r++) {
        for (int s = 0; s < m_sectors; s++) {
            const uint32_t tl = r * (m_sectors + 1) + s;
            const uint32_t tr = tl + 1;
            const uint32_t bl = tl + (m_sectors + 1);
            const uint32_t br = bl + 1;
            indices.insert(indices.end(), { tl, tr, bl, bl, tr, br });
        }
    }

    Regenerate(vertices, indices);
}

// ----- cylinder mesh -----
CylinderMesh::CylinderMesh(const float radius, const float height, const int sectors) : m_radius(radius), m_height(height), m_sectors(sectors) {
    CylinderMesh::BuildGeometry();
}

void CylinderMesh::BuildGeometry() {
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;

    const float hh = m_height * 0.5f;

    // side rings
    for (int cap = 0; cap <= 1; cap++) {
        const float y = (cap == 0) ? -hh : hh;
        for (int s = 0; s <= m_sectors; s++) {
            const float theta = 2.0f * PRIM_PI * static_cast<float>(s) / m_sectors;
            const float x = std::cos(theta);
            const float z = std::sin(theta);
            vertices.push_back({
                { x * m_radius, y, z * m_radius },
                { x, 0.0f, z },
                { static_cast<float>(s) / m_sectors, static_cast<float>(cap) }
            });
        }
    }

    // side indices
    for (int s = 0; s < m_sectors; s++) {
        const uint32_t bl = s;
        const uint32_t br = s + 1;
        const uint32_t tl = s + (m_sectors + 1);
        const uint32_t tr = tl + 1;
        indices.insert(indices.end(), { bl, tl, br, br, tl, tr });
    }

    // caps
    for (int cap = 0; cap < 2; cap++) {
        const float    y          = (cap == 0) ? -hh   : hh;
        const float    ny         = (cap == 0) ? -1.0f : 1.0f;
        const uint32_t centerIdx  = static_cast<uint32_t>(vertices.size());

        vertices.push_back({ {0, y, 0}, {0, ny, 0}, {0.5f, 0.5f} });

        const uint32_t ringStart = static_cast<uint32_t>(vertices.size());
        for (int s = 0; s <= m_sectors; s++) {
            const float theta = 2.0f * PRIM_PI * static_cast<float>(s) / m_sectors;
            const float x = std::cos(theta);
            const float z = std::sin(theta);
            vertices.push_back({
                { x * m_radius, y, z * m_radius },
                { 0, ny, 0 },
                { x * 0.5f + 0.5f, z * 0.5f + 0.5f }
            });
        }

        for (int s = 0; s < m_sectors; s++) {
            const uint32_t a = ringStart + s;
            const uint32_t b = ringStart + s + 1;
            // Bottom cap (ny = -1): normal points down, wind CW from above → a,b order
            // Top cap   (ny = +1): normal points up,   wind CCW from above → b,a order
            if (cap == 0) indices.insert(indices.end(), { centerIdx, a, b });
            else          indices.insert(indices.end(), { centerIdx, b, a });
        }
    }

    Regenerate(vertices, indices);
}

// ----- capsule mesh -----
CapsuleMesh::CapsuleMesh(const float radius, const float height, const int rings, const int sectors) : m_radius(radius), m_height(height), m_rings(rings), m_sectors(sectors) {
    CapsuleMesh::BuildGeometry();
}

void CapsuleMesh::BuildGeometry() {
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;

    const float cylinderHeight = std::max(0.0f, m_height - 2.0f * m_radius);
    const float halfCyl        = cylinderHeight * 0.5f;
    const int   rowSize        = m_sectors + 1;

    // top hemisphere  (phi 0 = north pole, phi PI/2 = equator)
    for (int r = 0; r <= m_rings; r++) {
        const float phi = (PRIM_PI * 0.5f) * static_cast<float>(r) / m_rings;
        for (int s = 0; s <= m_sectors; s++) {
            const float theta = 2.0f * PRIM_PI * static_cast<float>(s) / m_sectors;
            const float x = std::cos(phi) * std::cos(theta);
            const float y = std::sin(phi);
            const float z = std::cos(phi) * std::sin(theta);
            vertices.push_back({
                { x * m_radius, y * m_radius + halfCyl, z * m_radius },
                { x, y, z },
                { static_cast<float>(s) / m_sectors, static_cast<float>(r) / (2 * m_rings) + 0.5f }
            });
        }
    }

    // bottom hemisphere  (phi 0 = south pole, phi PI/2 = equator)
    for (int r = 0; r <= m_rings; r++) {
        const float phi = (PRIM_PI * 0.5f) * static_cast<float>(r) / m_rings;
        for (int s = 0; s <= m_sectors; s++) {
            const float theta = 2.0f * PRIM_PI * static_cast<float>(s) / m_sectors;
            const float x =  std::cos(phi) * std::cos(theta);
            const float y = -std::sin(phi);
            const float z =  std::cos(phi) * std::sin(theta);
            vertices.push_back({
                { x * m_radius, y * m_radius - halfCyl, z * m_radius },
                { x, y, z },
                { static_cast<float>(s) / m_sectors, static_cast<float>(m_rings - r) / (2 * m_rings) }
            });
        }
    }

    const int topOffset = 0;
    const int botOffset = (m_rings + 1) * rowSize;

    // top hemisphere indices
    // r=0 is the equator row, r=m_rings is the north pole row.
    // Winding matches the fixed sphere: { tl, tr, bl, bl, tr, br }
    for (int r = 0; r < m_rings; r++) {
        for (int s = 0; s < m_sectors; s++) {
            const uint32_t tl = topOffset + r * rowSize + s;
            const uint32_t tr = tl + 1;
            const uint32_t bl = tl + rowSize;
            const uint32_t br = bl + 1;
            indices.insert(indices.end(), { tl, bl, tr, tr, bl, br });
        }
    }

    // bottom hemisphere indices
    // r=0 is the equator row, r=m_rings is the south pole row.
    // Flipped vs. top to keep outward-facing normals: { tl, tr, bl, bl, tr, br }
    for (int r = 0; r < m_rings; r++) {
        for (int s = 0; s < m_sectors; s++) {
            const uint32_t tl = botOffset + r * rowSize + s;
            const uint32_t tr = tl + 1;
            const uint32_t bl = tl + rowSize;
            const uint32_t br = bl + 1;
            indices.insert(indices.end(), { tl, tr, bl, bl, tr, br });
        }
    }

    // cylinder band connecting top equator (r=0) to bottom equator (r=0)
    // top hemisphere r=0 IS the equator (phi=0 → y=sin(0)=0, offset upward by halfCyl)
    // bot hemisphere r=0 IS the equator (phi=0 → y=-sin(0)=0, offset downward by halfCyl)
    if (cylinderHeight > 0.0f) {
        const uint32_t topRingStart = topOffset;               // r=0 of top hemisphere
        const uint32_t botRingStart = botOffset;               // r=0 of bottom hemisphere
        for (int s = 0; s < m_sectors; s++) {
            const uint32_t tl = topRingStart + s;
            const uint32_t tr = tl + 1;
            const uint32_t bl = botRingStart + s;
            const uint32_t br = bl + 1;
            indices.insert(indices.end(), { bl, tl, br, br, tl, tr });
        }
    }

    Regenerate(vertices, indices);
}