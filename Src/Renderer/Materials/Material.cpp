
#include "Material.h"

#include <atomic>

#include "Core/ResourceManager.h"

REGISTER_PROPERTY_TYPE(Material)

uint64_t Material::GenerateResourceId() {
    static std::atomic<uint64_t> s_nextId {1};
    return s_nextId.fetch_add(1, std::memory_order_relaxed);
}

uint64_t Material::GetDefaultResourceId() {
    static const uint64_t s_defaultMaterialId = GenerateResourceId();
    return s_defaultMaterialId;
}

void Material::TouchResourceId() {
    m_resourceId = GenerateResourceId();
}

Material::Material() : m_resourceId(GetDefaultResourceId()) {
    AddProperty("name",
        [this]() -> PropertyValue { return GetName(); },
        [this](const PropertyValue& v) { SetName(std::get<std::string>(v)); }
    );

    AddProperty("albedo_color",
        [this]() -> PropertyValue { return glm::vec3(GetAlbedoColor()); },
        [this](const PropertyValue& v) { SetAlbedoColor(glm::vec4(std::get<glm::vec3>(v), m_albedoColor.a)); }
    );
    AddProperty("diffuse",
        [this]() -> PropertyValue { return m_diffuse ? m_diffuse->GetPath() : std::string(""); },
        [this](const PropertyValue& v) { SetDiffuse(std::get<std::string>(v)); }
    );

    AddProperty("metallic_value",
        [this]() -> PropertyValue { return GetMetallicValue(); },
        [this](const PropertyValue& v) { SetMetallicValue(std::get<float>(v)); }
    );
    AddProperty("metallic",
        [this]() -> PropertyValue { return m_metallic ? m_metallic->GetPath() : std::string(""); },
        [this](const PropertyValue& v) { SetMetallic(std::get<std::string>(v)); }
    );

    AddProperty("roughness_value",
        [this]() -> PropertyValue { return GetRoughnessValue(); },
        [this](const PropertyValue& v) { SetRoughnessValue(std::get<float>(v)); }
    );
    AddProperty("roughness",
        [this]() -> PropertyValue { return m_roughness ? m_roughness->GetPath() : std::string(""); },
        [this](const PropertyValue& v) { SetRoughness(std::get<std::string>(v)); }
    );

    AddProperty("ao_strength_value",
        [this]() -> PropertyValue { return GetAOStrength(); },
        [this](const PropertyValue& v) { SetAOStrength(std::get<float>(v)); }
    );
    AddProperty("ambient_occlusion",
        [this]() -> PropertyValue { return m_ambientOcclusion ? m_ambientOcclusion->GetPath() : std::string(""); },
        [this](const PropertyValue& v) { SetAmbientOcclusion(std::get<std::string>(v)); }
    );

    AddProperty("displacement_scale_value",
        [this]() -> PropertyValue { return GetDisplacementScale(); },
        [this](const PropertyValue& v) { SetDisplacementScale(std::get<float>(v)); }
    );
    AddProperty("displacement",
        [this]() -> PropertyValue { return m_displacement ? m_displacement->GetPath() : std::string(""); },
        [this](const PropertyValue& v) { SetDisplacement(std::get<std::string>(v)); }
    );

    AddProperty("normal_strength",
        [this]() -> PropertyValue { return GetNormalStrength(); },
        [this](const PropertyValue& v) { SetNormalStrength(std::get<float>(v)); }
    );
    AddProperty("normal",
        [this]() -> PropertyValue { return m_normal ? m_normal->GetPath() : std::string(""); },
        [this](const PropertyValue& v) { SetNormal(std::get<std::string>(v)); }
    );

    AddProperty("emission_strength",
        [this]() -> PropertyValue { return GetEmissionStrength(); },
        [this](const PropertyValue& v) { SetEmissionStrength(std::get<float>(v)); }
    );
    AddProperty("emissive",
        [this]() -> PropertyValue { return m_emissive ? m_emissive->GetPath() : std::string(""); },
        [this](const PropertyValue& v) { SetEmissive(std::get<std::string>(v)); }
    );

    AddProperty("blend_mode",
        [this]() -> PropertyValue {
            switch (m_blendMode) {
                case BlendMode::Opaque: return std::string("Opaque");
                case BlendMode::AlphaBlend: return std::string("AlphaBlend");
                case BlendMode::AlphaScissor: return std::string("AlphaScissor");
                default: return std::string("Opaque");
            }
        },
        [this](const PropertyValue& v) {
            const std::string& str = std::get<std::string>(v);
            if (str == "AlphaBlend") SetBlendMode(BlendMode::AlphaBlend);
            else if (str == "AlphaScissor") SetBlendMode(BlendMode::AlphaScissor);
            else SetBlendMode(BlendMode::Opaque);
        }
    );
    AddProperty("alpha_scissor_threshold",
        [this]() -> PropertyValue { return GetAlphaScissorThreshold(); },
        [this](const PropertyValue& v) { SetAlphaScissorThreshold(std::get<float>(v)); }
    );

    AddProperty("uv_scale",
        [this]() -> PropertyValue { return GetUVScale(); },
        [this](const PropertyValue& v) { SetUVScale(std::get<glm::vec2>(v)); }
    );
    AddProperty("uv_offset",
        [this]() -> PropertyValue { return GetUVOffset(); },
        [this](const PropertyValue& v) { SetUVOffset(std::get<glm::vec2>(v)); }
    );

    AddProperty("double_sided",
        [this]() -> PropertyValue { return GetDoubleSided(); },
        [this](const PropertyValue& v) { SetDoubleSided(std::get<bool>(v)); }
    );
    AddProperty("cast_shadows",
        [this]() -> PropertyValue { return m_castShadows; },
        [this](const PropertyValue& v) { SetCastShadows(std::get<bool>(v)); }
    );
}

Material::Material(const Material& other) : Material() {
    m_resourceId = other.m_resourceId;
    m_name = other.m_name;
    m_filePath = other.m_filePath;
    m_albedoColor = other.m_albedoColor;
    m_metallicValue = other.m_metallicValue;
    m_roughnessValue = other.m_roughnessValue;
    m_aoStrength = other.m_aoStrength;
    m_normalStrength = other.m_normalStrength;
    m_emissionStrength = other.m_emissionStrength;
    m_emissionColor = other.m_emissionColor;
    m_displacementScale = other.m_displacementScale;
    m_useDisplacement = other.m_useDisplacement;
    m_castShadows = other.m_castShadows;
    m_doubleSided = other.m_doubleSided;
    m_dirty = other.m_dirty;
    m_blendMode = other.m_blendMode;
    m_alphaScissorThreshold = other.m_alphaScissorThreshold;
    m_uvScale = other.m_uvScale;
    m_uvOffset = other.m_uvOffset;
    m_shader = other.m_shader;
    m_diffuse = other.m_diffuse;
    m_ambientOcclusion = other.m_ambientOcclusion;
    m_normal = other.m_normal;
    m_roughness = other.m_roughness;
    m_metallic = other.m_metallic;
    m_displacement = other.m_displacement;
    m_emissive = other.m_emissive;
}

Material& Material::operator=(const Material& other) {
    if (this == &other) {
        return *this;
    }

    m_resourceId = other.m_resourceId;
    m_name = other.m_name;
    m_filePath = other.m_filePath;
    m_albedoColor = other.m_albedoColor;
    m_metallicValue = other.m_metallicValue;
    m_roughnessValue = other.m_roughnessValue;
    m_aoStrength = other.m_aoStrength;
    m_normalStrength = other.m_normalStrength;
    m_emissionStrength = other.m_emissionStrength;
    m_emissionColor = other.m_emissionColor;
    m_displacementScale = other.m_displacementScale;
    m_useDisplacement = other.m_useDisplacement;
    m_castShadows = other.m_castShadows;
    m_doubleSided = other.m_doubleSided;
    m_dirty = other.m_dirty;
    m_blendMode = other.m_blendMode;
    m_alphaScissorThreshold = other.m_alphaScissorThreshold;
    m_uvScale = other.m_uvScale;
    m_uvOffset = other.m_uvOffset;
    m_shader = other.m_shader;
    m_diffuse = other.m_diffuse;
    m_ambientOcclusion = other.m_ambientOcclusion;
    m_normal = other.m_normal;
    m_roughness = other.m_roughness;
    m_metallic = other.m_metallic;
    m_displacement = other.m_displacement;
    m_emissive = other.m_emissive;
    return *this;
}

/*
 * Material textures layout order:
 *
 * diffuse / albedo == slot 0
 * normal == slot 1
 * metallic == slot 2
 * roughness == slot 3
 * ao == slot 4
 * emissive == slot 5
 * displacement == slot 6
 *
 */
void Material::Bind(const Shader &shader) const {
    shader.SetVector4("u_AlbedoColor", m_albedoColor);
    shader.SetFloat("u_MetallicValue", m_metallicValue);
    shader.SetFloat("u_RoughnessValue", m_roughnessValue);
    shader.SetFloat("u_AOStrength", m_aoStrength);
    shader.SetFloat("u_NormalStrength", m_normalStrength);
    shader.SetFloat("u_EmissionStrength", m_emissionStrength);
    shader.SetVector3("u_EmissionColor", m_emissionColor);
    shader.SetFloat("u_DisplacementScale", m_displacementScale);
    shader.SetBool("u_AlphaScissor", m_blendMode == BlendMode::AlphaScissor);
    shader.SetFloat("u_AlphaScissorThreshold", m_alphaScissorThreshold);
    shader.SetVector2("u_UVScale", m_uvScale);
    shader.SetVector2("u_UVOffset", m_uvOffset);
    shader.SetBool("u_DoubleSided", m_doubleSided);

    const bool packed = m_metallic && m_roughness && m_metallic == m_roughness;
    shader.SetBool("u_HasMetallicRoughnessPacked", packed);

    if (m_diffuse) {
        m_diffuse->Bind(0);
        shader.SetInt("u_Diffuse", 0);
        shader.SetBool("u_HasDiffuse", true);
    } else {
        shader.SetBool("u_HasDiffuse", false);
    }

    if (m_normal) {
        m_normal->Bind(1);
        shader.SetInt("u_Normal", 1);
        shader.SetBool("u_HasNormal", true);
    } else {
        shader.SetBool("u_HasNormal", false);
    }


    if (m_metallic) {
        m_metallic->Bind(2);
        shader.SetInt("u_Metallic", 2);
        shader.SetBool("u_HasMetallic", !packed);
    } else {
        shader.SetBool("u_HasMetallic", false);
    }

    if (!packed && m_roughness) {
        m_roughness->Bind(3);
        shader.SetInt("u_Roughness", 3);
        shader.SetBool("u_HasRoughness", true);
    } else {
        shader.SetBool("u_HasRoughness", false);
    }

    if (m_ambientOcclusion) {
        m_ambientOcclusion->Bind(4);
        shader.SetInt("u_AmbientOcclusion", 4);
        shader.SetBool("u_HasAmbientOcclusion", true);
    } else {
        shader.SetBool("u_HasAmbientOcclusion", false);
    }

    if (m_emissive) {
        m_emissive->Bind(5);
        shader.SetInt("u_Emissive", 5);
        shader.SetBool("u_HasEmissive", true);
    } else {
        shader.SetBool("u_HasEmissive", false);
    }

    if (m_displacement) {
        m_displacement->Bind(6);
        shader.SetInt("u_Displacement", 6);
        shader.SetBool("u_HasDisplacement", true);
    } else {
        shader.SetBool("u_HasDisplacement", false);
    }
}

void Material::SetName(const std::string &name) {
    m_name = name;
    SetDirty(true);
}

void Material::SetFilePath(const std::string &path) {
    m_filePath = path;
    SetDirty(true);
}

void Material::SetAlbedoColor(const glm::vec4 &color) {
    m_albedoColor = color;
    SetDirty(true);
}

void Material::SetMetallicValue(const float value) {
    m_metallicValue = value;
    SetDirty(true);
}

void Material::SetRoughnessValue(const float value) {
    m_roughnessValue = value;
    SetDirty(true);
}

void Material::SetAOStrength(const float value) {
    m_aoStrength = value;
    SetDirty(true);
}

void Material::SetNormalStrength(const float value) {
    m_normalStrength = value;
    SetDirty(true);
}

void Material::SetEmissionStrength(const float value) {
    m_emissionStrength = value;
    SetDirty(true);
}

void Material::SetEmissionColor(const glm::vec3 &color) {
    m_emissionColor = color;
    SetDirty(true);
}

void Material::SetDisplacementScale(const float value) {
    m_displacementScale = value;
    SetDirty(true);
}

void Material::SetUseDisplacement(const bool value) {
    m_useDisplacement = value;
    SetDirty(true);
}

void Material::SetCastShadows(const bool value) {
    m_castShadows = value;
    SetDirty(true);
}

void Material::SetDoubleSided(const bool value) {
    m_doubleSided = value;
    SetDirty(true);
}

void Material::SetDirty(const bool value) {
    m_dirty = value;
    if (value) {
        TouchResourceId();
    }
}

void Material::SetBlendMode(const BlendMode mode) {
    m_blendMode = mode;
    SetDirty(true);
}

void Material::SetAlphaScissorThreshold(const float value) {
    m_alphaScissorThreshold = value;
    SetDirty(true);
}

void Material::SetUVScale(const glm::vec2 &scale) {
    m_uvScale = scale;
    SetDirty(true);
}

void Material::SetUVOffset(const glm::vec2 &offset) {
    m_uvOffset = offset;
    SetDirty(true);
}

void Material::SetShader(const std::shared_ptr<Shader> &shader) {
    m_shader = shader;
    SetDirty(true);
}

std::shared_ptr<Shader> Material::GetShader() const {
    if (m_shader != nullptr) {
        return m_shader;
    }

    return ResourceManager::Get().GetDefaultShader();
}

void Material::SetDiffuse(const std::string &path) {
    m_diffuse = ResourceManager::Get().LoadTexture(path, true);
    SetDirty(true);
}

void Material::SetAmbientOcclusion(const std::string &path) {
    m_ambientOcclusion = ResourceManager::Get().LoadTexture(path, false);
    SetDirty(true);
}

void Material::SetNormal(const std::string &path) {
    m_normal = ResourceManager::Get().LoadTexture(path, false);
    SetDirty(true);
}

void Material::SetRoughness(const std::string &path) {
    m_roughness = ResourceManager::Get().LoadTexture(path, false);
    SetDirty(true);
}

void Material::SetMetallic(const std::string &path) {
    m_metallic = ResourceManager::Get().LoadTexture(path, false);
    SetDirty(true);
}

void Material::SetDisplacement(const std::string &path) {
    m_displacement = ResourceManager::Get().LoadTexture(path, false);
    SetDirty(true);
}

void Material::SetEmissive(const std::string &path) {
    m_emissive = ResourceManager::Get().LoadTexture(path, true);
    SetDirty(true);
}
