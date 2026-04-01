
#include "Skybox.h"

#include "Core/ResourceManager.h"
#include "Materials/EquirectToCubemap.h"
#include "Meshes/CubeGeometry.h"

Skybox::Skybox(std::vector<std::string>& faces) {
    m_texture = std::make_shared<Texture>(faces);
    m_shader = ResourceManager::Get().LoadShader("SkyboxShader", "skybox.vert", "skybox.frag");
    m_vao = CubeGeometry::CreateVao();
}

Skybox::Skybox(const std::string &hdrPath, const int faceSize) {
    m_texture = EquirectToCubemap::Convert(hdrPath, faceSize);
    m_shader = ResourceManager::Get().LoadShader("SkyboxShader", "skybox.vert", "skybox.frag");
    m_vao = CubeGeometry::CreateVao();
}

Skybox::~Skybox() {
    delete m_vao;
}

void Skybox::Draw(const glm::mat4& view, const glm::mat4& projection) const {
    const glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    m_shader->Bind();
    m_shader->SetMatrix4("u_View", viewNoTranslation);
    m_shader->SetMatrix4("u_Projection", projection);
    m_shader->SetInt("u_Skybox", 0);

    m_texture->Bind(0);
    m_vao->Bind();
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);  // restore
}