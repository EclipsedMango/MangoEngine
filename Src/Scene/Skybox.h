
#ifndef MANGORENDERING_SKYBOX_H
#define MANGORENDERING_SKYBOX_H
#include <array>
#include <cstdint>
#include <string>

#include "Renderer/Shader.h"
#include "Renderer/Texture.h"


class Skybox {
public:
    explicit Skybox(const std::array<std::string, 6>& facePaths); // +X -X +Y -Y +Z -Z
    ~Skybox();

    Skybox(const Skybox&) = delete;
    Skybox& operator=(const Skybox&) = delete;

    void Draw() const;

    [[nodiscard]] uint32_t GetTextureID() const { return m_texture->GetID(); }

private:
    void BuildMesh();

    Texture* m_texture = nullptr;
    uint32_t m_vao     = 0;
    uint32_t m_vbo     = 0;
    Shader*  m_shader  = nullptr;
};


#endif //MANGORENDERING_SKYBOX_H