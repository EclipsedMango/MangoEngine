#include "Skybox.h"

#include <stdexcept>
#include "glad/gl.h"

static constexpr float cubeVerts[] = {
    -1, 1,-1,  -1,-1,-1,   1,-1,-1,   1,-1,-1,   1, 1,-1,  -1, 1,-1,
    -1,-1, 1,  -1,-1,-1,  -1, 1,-1,  -1, 1,-1,  -1, 1, 1,  -1,-1, 1,
     1,-1,-1,   1,-1, 1,   1, 1, 1,   1, 1, 1,   1, 1,-1,   1,-1,-1,
    -1,-1, 1,  -1, 1, 1,   1, 1, 1,   1, 1, 1,   1,-1, 1,  -1,-1, 1,
    -1, 1,-1,   1, 1,-1,   1, 1, 1,   1, 1, 1,  -1, 1, 1,  -1, 1,-1,
    -1,-1,-1,  -1,-1, 1,   1,-1,-1,   1,-1,-1,  -1,-1, 1,   1,-1, 1
};

Skybox::Skybox(const std::array<std::string, 6>& facePaths) {
    m_shader  = new Shader("../Assets/Shaders/skybox.vert", "../Assets/Shaders/skybox.frag");
    m_texture = new Texture(facePaths);
    BuildMesh();
}

Skybox::~Skybox() {
    delete m_texture;
    delete m_shader;
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
}

void Skybox::BuildMesh() {
    glCreateVertexArrays(1, &m_vao);
    glCreateBuffers(1, &m_vbo);

    glNamedBufferStorage(m_vbo, sizeof(cubeVerts), cubeVerts, 0);

    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, 3 * sizeof(float));
    glEnableVertexArrayAttrib(m_vao, 0);
    glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(m_vao, 0, 0);
}

void Skybox::Draw() const {
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    m_shader->Bind();
    m_texture->Bind(0);
    m_shader->SetInt("u_Skybox", 0);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
}
