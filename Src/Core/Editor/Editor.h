
#ifndef MANGORENDERING_EDITOR_H
#define MANGORENDERING_EDITOR_H

#include "EditorCameraController.h"
#include "GizmoSystem.h"
#include "../Core.h"
#include "UI/InspectorPanel.h"
#include "UI/SceneTreePanel.h"
#include "UI/ViewportWindow.h"

class ViewportWindow;

class Editor {
public:
    explicit Editor(Node3d* scene);
    ~Editor();

    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;

    enum class State { Editing, Playing, Paused };

    void Run();

    [[nodiscard]] Core& GetCore() { return m_core; }
    [[nodiscard]] SceneTreePanel& GetSceneTree() { return m_sceneTree; }
    [[nodiscard]] GizmoSystem& GetGizmoSystem() { return m_gizmoSystem; }
    [[nodiscard]] State GetState() const { return m_state; }

    void SetGameCamera(std::unique_ptr<CameraNode3d> camera) { m_gameCamera = std::move(camera); }

private:
    // panels
    void DrawMenuBar();
    void DrawContentBrowser();

    // helpers
    void OnPlay();
    void OnPause();
    void OnStop();

    [[nodiscard]] bool IsAnyViewportLooking() const;
    std::vector<std::unique_ptr<ViewportWindow>> m_viewports;

    // cameras
    std::unique_ptr<CameraNode3d> m_gameCamera {};

    Core m_core;
    InspectorPanel m_inspector;
    SceneTreePanel m_sceneTree;
    GizmoSystem m_gizmoSystem;

    State m_state = State::Editing;

    float m_cpuTime = 0.0f;
};

#endif //MANGORENDERING_EDITOR_H