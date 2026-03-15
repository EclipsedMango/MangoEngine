
#include "Editor.h"

#include <functional>

#include "imgui.h"
#include "Nodes/MeshNode3d.h"
#include "Nodes/CameraNode3d.h"
#include "Nodes/Lights/DirectionalLightNode3d.h"
#include "Nodes/Lights/PointLightNode3d.h"
#include "Nodes/Lights/SpotLightNode3d.h"

Editor::Editor(Node3d* scene) : m_core(scene) {
    m_core.Init();
}

void Editor::Run() {
    uint64_t lastTime = SDL_GetTicksNS();

    while (m_core.GetActiveWindow()->IsOpen()) {
        const uint64_t now = SDL_GetTicksNS();
        const float deltaTime = (now - lastTime) / 1e9f;
        lastTime = now;

        m_core.PollEvents();

        if (m_state == State::Playing) {
            m_core.StepFrame(deltaTime);
        } else {
            m_core.GetScene()->UpdateWorldTransform();
        }

        Core::BeginImGuiFrame();

        DrawMenuBar();
        DrawSceneTree(m_core.GetScene());
        DrawInspector();
        DrawContentBrowser();

        m_core.RenderScene();
        Core::EndImGuiFrame();
        m_core.SwapBuffers();
    }
}


void Editor::DrawMenuBar() {
    if (!ImGui::BeginMainMenuBar()) return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Scene"))  { /* TODO */ }
        if (ImGui::MenuItem("Open Scene")) { /* TODO */ }
        if (ImGui::MenuItem("Save Scene")) { /* TODO */ }
        ImGui::Separator();
        if (ImGui::MenuItem("Exit")) m_core.GetActiveWindow()->Close();
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Undo")) { /* TODO */ }
        if (ImGui::MenuItem("Redo")) { /* TODO */ }
        ImGui::EndMenu();
    }

    const float buttonWidth = 60.0f;
    const float spacing     = 4.0f;
    const float totalWidth  = buttonWidth * 3 + spacing * 2;
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - totalWidth) * 0.5f + ImGui::GetCursorPosX());

    const bool isPlaying = m_state == State::Playing;
    const bool isPaused  = m_state == State::Paused;
    const bool isEditing = m_state == State::Editing;

    // play
    if (isPlaying) ImGui::BeginDisabled();
    if (ImGui::Button("Play", {buttonWidth, 0})) OnPlay();
    if (isPlaying) ImGui::EndDisabled();

    ImGui::SameLine(0, spacing);

    // pause
    if (isEditing) ImGui::BeginDisabled();
    if (ImGui::Button("Pause", {buttonWidth, 0})) OnPause();
    if (isEditing) ImGui::EndDisabled();

    ImGui::SameLine(0, spacing);

    // stop
    if (isEditing) ImGui::BeginDisabled();
    if (ImGui::Button("Stop", {buttonWidth, 0})) OnStop();
    if (isEditing) ImGui::EndDisabled();

    ImGui::EndMainMenuBar();
}

void Editor::DrawSceneTree(Node3d* node) {
    ImGui::Begin("Scene Tree");
    std::function<void(Node3d*)> drawNode = [&](Node3d* n) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (n == m_selectedNode)      flags |= ImGuiTreeNodeFlags_Selected;
        if (n->GetChildren().empty()) flags |= ImGuiTreeNodeFlags_Leaf;

        // use pointer as unique id, label as node type
        const char* label = "Node3d";
        if      (dynamic_cast<MeshNode3d*>(n))              label = "MeshNode3d";
        else if (dynamic_cast<DirectionalLightNode3d*>(n))  label = "DirectionalLight";
        else if (dynamic_cast<PointLightNode3d*>(n))        label = "PointLight";
        else if (dynamic_cast<SpotLightNode3d*>(n))         label = "SpotLight";
        else if (dynamic_cast<CameraNode3d*>(n))            label = "Camera";

        const bool open = ImGui::TreeNodeEx((void*)n, flags, "%s", label);
        if (ImGui::IsItemClicked()) m_selectedNode = n;

        if (open) {
            for (auto* child : n->GetChildren()) drawNode(child);
            ImGui::TreePop();
        }
    };

    drawNode(node);
    ImGui::End();
}

void Editor::DrawInspector() const {
    ImGui::Begin("Inspector");

    if (!m_selectedNode) {
        ImGui::TextDisabled("No node selected");
        ImGui::End();
        return;
    }

    // transform
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        glm::vec3 pos = m_selectedNode->GetPosition();
        glm::vec3 rot = m_selectedNode->GetRotation();
        glm::vec3 scl = m_selectedNode->GetScale();

        if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) m_selectedNode->SetPosition(pos);
        if (ImGui::DragFloat3("Rotation", &rot.x, 0.5f)) m_selectedNode->SetRotation(rot);
        if (ImGui::DragFloat3("Scale",    &scl.x, 0.1f)) m_selectedNode->SetScale(scl);
    }

    // mesh node
    if (auto* mesh = dynamic_cast<MeshNode3d*>(m_selectedNode)) {
        if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& mat = mesh->GetMaterial();

            glm::vec4 color = mat.GetAlbedoColor();
            if (ImGui::ColorEdit4("Albedo", &color.x)) mat.SetAlbedoColor(color);

            float metallic  = mat.GetMetallicValue();
            float roughness = mat.GetRoughnessValue();
            if (ImGui::SliderFloat("Metallic",  &metallic,  0.0f, 1.0f)) mat.SetMetallicValue(metallic);
            if (ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f)) mat.SetRoughnessValue(roughness);

            bool doubleSided = mat.GetDoubleSided();
            if (ImGui::Checkbox("Double Sided", &doubleSided)) mat.SetDoubleSided(doubleSided);

            bool castShadows = mat.GetCastShadows();
            if (ImGui::Checkbox("Cast Shadows", &castShadows)) mat.SetCastShadows(castShadows);
        }
    }

    // point light node
    if (auto* pl = dynamic_cast<PointLightNode3d*>(m_selectedNode)) {
        if (ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            // TODO: expose light properties via PointLightNode3d getters/setters
            ImGui::TextDisabled("Light properties coming soon");
        }
    }

    // directional light node
    if (auto* dl = dynamic_cast<DirectionalLightNode3d*>(m_selectedNode)) {
        if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::TextDisabled("Light properties coming soon");
        }
    }

    ImGui::End();
}


void Editor::DrawContentBrowser() {
    ImGui::Begin("Content Browser");
    ImGui::TextDisabled("Content browser coming soon");
    ImGui::End();
}

void Editor::OnPlay() {
    // TODO: serialize scene state here so we can restore on stop
    m_state = State::Playing;
}

void Editor::OnPause() {
    m_state = m_state == State::Paused ? State::Playing : State::Paused;
}

void Editor::OnStop() {
    // TODO: restore serialized scene state here
    m_state = State::Editing;
}