
#include "CameraNode3d.h"

REGISTER_NODE_TYPE(CameraNode3d)

CameraNode3d::CameraNode3d() : CameraNode3d(glm::vec3(0.0f), 60.0f, 16.0f / 9.0f) {}

CameraNode3d::CameraNode3d(const glm::vec3 position, const float fov, const float aspectRatio) : m_fov(fov), m_aspectRatio(aspectRatio) {
    SetPosition(position);
    SetRotation(RotationFromYawPitch(-90.0f, 0.0f));
    SetName("CameraNode3d");

    AddProperty("fov",
        [this]() -> PropertyValue { return m_fov; },
        [this](const PropertyValue& v) { m_fov = std::get<float>(v); }
    );
    AddProperty("near_plane",
        [this]() -> PropertyValue { return m_nearPlane; },
        [this](const PropertyValue& v) { SetNearPlane(std::get<float>(v)); }
    );
    AddProperty("far_plane",
        [this]() -> PropertyValue { return m_farPlane; },
        [this](const PropertyValue& v) { SetFarPlane(std::get<float>(v)); }
    );
    AddProperty("yaw",
        [this]() -> PropertyValue { return GetYaw(); },
        [this](const PropertyValue& v) { SetYaw(std::get<float>(v)); }
    );
    AddProperty("pitch",
        [this]() -> PropertyValue { return GetPitch(); },
        [this](const PropertyValue& v) { SetPitch(std::get<float>(v)); }
    );
    AddProperty("is_main_camera",
        [this]() -> PropertyValue { return m_isGameCamera; },
        [this](const PropertyValue& v) { SetAsGameCamera(std::get<bool>(v)); }
    );
}

std::unique_ptr<Node3d> CameraNode3d::Clone() {
    auto clone = std::make_unique<CameraNode3d>(GetPosition(), m_fov, m_aspectRatio);

    CopyBaseStateTo(*clone);

    clone->SetNearPlane(m_nearPlane);
    clone->SetFarPlane(m_farPlane);
    clone->SetAspectRatio(m_aspectRatio);
    clone->SetAsGameCamera(m_isGameCamera);

    for (Node3d* child : GetChildren()) {
        clone->AddChild(child->Clone());
    }

    return clone;
}

void CameraNode3d::SetYaw(const float yaw) {
    SetYawPitch(yaw, GetPitch());
}

void CameraNode3d::SetPitch(const float pitch) {
    SetYawPitch(GetYaw(), pitch);
}

void CameraNode3d::SetViewMatrixOverride(const glm::mat4 &viewMatrix) {
    m_viewOverride = viewMatrix;
    m_hasViewOverride = true;
}

void CameraNode3d::SetProjectionMatrixOverride(const glm::mat4 &projMatrix) {
    m_projOverride = projMatrix;
    m_hasProjectionOverride = true;
}

void CameraNode3d::ClearViewMatrixOverride() {
    m_hasViewOverride = false;
}

void CameraNode3d::ClearProjectionMatrixOverride() {
    m_hasProjectionOverride = false;
}

glm::mat4 CameraNode3d::GetViewMatrix() const {
    if (m_hasViewOverride) {
        return m_viewOverride;
    }

    const glm::vec3 worldPosition = GetWorldPosition();
    const glm::vec3 front = GetFront();
    const glm::vec3 up = GetUp();

    return glm::lookAt(worldPosition, worldPosition + front, up);
}

glm::mat4 CameraNode3d::GetProjectionMatrix() const {
    if (m_hasProjectionOverride) {
        return m_projOverride;
    }

    return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
}

glm::vec3 CameraNode3d::GetFront() const {
    return TransformLocalDirection(glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::vec3 CameraNode3d::GetRight() const {
    return TransformLocalDirection(glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 CameraNode3d::GetUp() const {
    return TransformLocalDirection(glm::vec3(0.0f, 1.0f, 0.0f));
}

float CameraNode3d::GetYaw() const {
    return glm::degrees(std::atan2(GetLocalFront().z, GetLocalFront().x));
}

float CameraNode3d::GetPitch() const {
    return glm::degrees(std::asin(glm::clamp(GetLocalFront().y, -1.0f, 1.0f)));
}

void CameraNode3d::Rotate(const float yawDelta, const float pitchDelta) {
    const float yaw = GetYaw() + yawDelta;
    const float pitch = GetPitch() + pitchDelta;

    SetYawPitch(yaw, pitch);
}

void CameraNode3d::SetYawPitch(float yaw, float pitch) {
    const float clampedPitch = glm::clamp(pitch, -89.0f, 89.0f);
    SetRotation(RotationFromYawPitch(yaw, clampedPitch));
}

glm::vec3 CameraNode3d::GetLocalFront() const {
    return glm::normalize(GetRotation() * glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::vec3 CameraNode3d::TransformLocalDirection(const glm::vec3 &localDir) const {
    glm::vec3 dir = GetRotation() * localDir;

    // I intentionally do not use world matrix here because during _process
    // the world matrix may not have been rebuilt yet after Rotate()
    if (const Node3d* parent = GetParent()) {
        dir = glm::mat3(parent->GetWorldMatrix()) * dir;
    }

    const float len = glm::length(dir);
    if (len <= 0.00001f) {
        return {0.0f, 0.0f, -1.0f};
    }

    return dir / len;
}

glm::quat CameraNode3d::RotationFromYawPitch(float yaw, float pitch) {
    const float yawRad = glm::radians(yaw);
    const float pitchRad = glm::radians(pitch);

    const glm::vec3 front = glm::normalize(glm::vec3(
        std::cos(yawRad) * std::cos(pitchRad),
        std::sin(pitchRad),
        std::sin(yawRad) * std::cos(pitchRad)
    ));

    constexpr glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::cross(front, worldUp);

    if (glm::length(right) <= 0.00001f) {
        right = glm::vec3(1.0f, 0.0f, 0.0f);
    } else {
        right = glm::normalize(right);
    }

    const glm::vec3 up = glm::normalize(glm::cross(right, front));

    // GLM matrices are column-major
    //
    // Local +X = right
    // Local +Y = up
    // Local -Z = front
    //
    // Therefore local +Z = -front.
    const glm::mat3 rotationMatrix(right, up, -front);
    return glm::normalize(glm::quat_cast(rotationMatrix));
}
