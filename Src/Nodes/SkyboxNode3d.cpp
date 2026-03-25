
#include "SkyboxNode3d.h"

REGISTER_NODE_TYPE(SkyboxNode3d)

SkyboxNode3d::SkyboxNode3d() {
    SetName("SkyboxNode3d");
    RegisterProperties();
}

SkyboxNode3d::SkyboxNode3d(std::vector<std::string> faces) : m_skybox(std::make_unique<Skybox>(faces)) {
    SetName("SkyboxNode3d");
    RegisterProperties();
}

SkyboxNode3d::SkyboxNode3d(const std::string &hdrPath) : m_skybox(std::make_unique<Skybox>(hdrPath)) {
    SetName("SkyboxNode3d");
    m_hdrPath = hdrPath;
    RegisterProperties();
}

void SkyboxNode3d::RegisterProperties() {
    AddProperty("hdr_path",
        [this]() -> PropertyValue { return m_hdrPath; },
        [this](const PropertyValue& v) { SetHdrPath(std::get<std::string>(v)); }
    );
    AddProperty("intensity",
    [this]() -> PropertyValue { return m_intensity; },
    [this](const PropertyValue& v) { SetIntensity(std::get<float>(v)); }
    );
    AddProperty("specular_intensity",
        [this]() -> PropertyValue { return m_specularIntensity; },
        [this](const PropertyValue& v) { SetSpecularIntensity(std::get<float>(v)); }
    );
}

void SkyboxNode3d::SetHdrPath(const std::string &path) {
    m_hdrPath = path;
    if (!path.empty()) {
        m_skybox = std::make_unique<Skybox>(m_hdrPath);
    }
}

void SkyboxNode3d::SetIntensity(const float intensity) {
    m_intensity = intensity;
}

void SkyboxNode3d::SetSpecularIntensity(const float intensity) {
    m_specularIntensity = intensity;
}
