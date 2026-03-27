
#include "Editor.h"

#include "Utils/DebugDraw.h"
#include "../Input.h"
#include "Core/PackedScene.h"
#include "Core/ResourceManager.h"
#include "glm/gtc/type_ptr.inl"
#include "UI/ViewportWindow.h"

Editor::Editor(Node3d* scene) : m_core(scene), m_inspector(this), m_sceneTree(this) {
    m_core.Init();
    m_viewports.push_back(std::make_unique<ViewportWindow>(this, "Viewport 1"));

    m_core.SetEditorCamera(m_viewports[0]->GetCamera());
    m_core.SetCameraMode(Core::CameraMode::Editor);

    SDL_SetWindowRelativeMouseMode(m_core.GetActiveWindow()->GetSDLWindow(), false);
    Input::SetMouseDeltaEnabled(false);
}

Editor::~Editor() {
    for (auto& viewport : m_viewports) {
        viewport.reset();
    }

    SDL_SetWindowRelativeMouseMode(m_core.GetActiveWindow()->GetSDLWindow(), false);
    Input::SetMouseDeltaEnabled(false);
}

void Editor::Run() {
    uint64_t lastTime = SDL_GetTicksNS();

    while (m_core.GetActiveWindow()->IsOpen()) {
        const uint64_t now = SDL_GetTicksNS();
        const float deltaTime = (now - lastTime) / 1e9f;
        lastTime = now;

        const uint64_t cpuStart = SDL_GetTicksNS();

        m_core.PollEvents();

        if (Input::IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
            m_sceneTree.ClearSelection();
        }

        if (Input::IsKeyJustPressedWithMod(SDL_SCANCODE_D, SDL_SCANCODE_LCTRL)) {
            m_sceneTree.DuplicateSelectedNodes();
        }

        const bool isLooking = IsAnyViewportLooking();

        m_gizmoSystem.HandleShortcuts(isLooking);
        m_core.SubmitFrameRenderables();
        Core::BeginImGuiFrame();

        if (isLooking) {
            ImGui::GetIO().MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
        }

        if (m_state != State::Playing) {
            for (const auto& viewport : m_viewports) {
                viewport->Update(deltaTime);
            }
            m_core.GetScene()->UpdateWorldTransform();
        } else {
            m_core.StepFrame(deltaTime);
        }

        DrawMenuBar();
        m_sceneTree.DrawSceneTree(m_core.GetScene());
        m_inspector.DrawInspector(m_sceneTree.GetSelectedNode());
        DrawContentBrowser();

        for (const auto& viewport : m_viewports) {
            viewport->Draw();
        }

        if (Input::IsKeyJustPressed(SDL_SCANCODE_DELETE)) {
            m_sceneTree.DeleteSelectedNodes();
        }

        Core::EndImGuiFrame();

        const uint64_t cpuEnd = SDL_GetTicksNS();
        m_cpuTime = static_cast<float>(cpuEnd - cpuStart) / 1000000.0f;

        m_core.SwapBuffers();
    }
}

void Editor::DrawMenuBar() {
    if (!ImGui::BeginMainMenuBar()) return;

    const float fps = ImGui::GetIO().Framerate;
    ImGui::Text("FPS: %.0f (%.2f ms) | CPU: %.2f ms", fps, 1000.0f / fps, m_cpuTime);

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Scene"))  { /* TODO */ }
        if (ImGui::MenuItem("Open Scene")) {
            PackedScene scene = PackedScene::LoadFromFile("../Assets/john.yml");
            Node3d* node = scene.Instantiate();
            m_core.ChangeScene(node);
        }
        if (ImGui::MenuItem("Save Scene")) {
            PackedScene* scene = new PackedScene(m_core.GetScene());
            scene->SaveToFile("../Assets/john.yml");
        }
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
    const float spacing = 4.0f;
    const float totalWidth = buttonWidth * 3 + spacing * 2;
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - totalWidth) * 0.5f + ImGui::GetCursorPosX());

    const bool isPlaying = m_state == State::Playing;
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

void Editor::DrawContentBrowser() {
    ImGui::Begin("Content Browser");
    ImGui::TextDisabled("Content browser coming soon");
    ImGui::End();
}

void Editor::OnPlay() {
    m_state = State::Playing;
    m_core.SetCameraMode(Core::CameraMode::Game);

    for (const auto& vp : m_viewports) {
        vp->GetCameraController()->CancelLook();
    }

    SDL_SetWindowRelativeMouseMode(m_core.GetActiveWindow()->GetSDLWindow(), false);
    Input::SetMouseDeltaEnabled(false);
}

void Editor::OnPause() {
    if (m_state == State::Playing) {
        m_state = State::Paused;
        m_core.SetCameraMode(Core::CameraMode::Editor);
    } else if (m_state == State::Paused) {
        m_state = State::Playing;
        m_core.SetCameraMode(Core::CameraMode::Game);
    }
}

void Editor::OnStop() {
    // TODO: restore scene using deserialization

    m_state = State::Editing;
    m_core.SetCameraMode(Core::CameraMode::Editor);

    for (const auto& vp : m_viewports) {
        vp->GetCameraController()->CancelLook();
    }

    SDL_SetWindowRelativeMouseMode(m_core.GetActiveWindow()->GetSDLWindow(), false);
    Input::SetMouseDeltaEnabled(false);
}

bool Editor::IsAnyViewportLooking() const {
    for (const auto& vp : m_viewports) {
        if (vp->GetCameraController()->IsLooking()) {
            return true;
        }
    }

    return false;
}
