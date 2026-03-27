
#ifndef MANGORENDERING_VIEWPORTWINDOW_H
#define MANGORENDERING_VIEWPORTWINDOW_H
#include <memory>
#include <string>

#include "imgui.h"

class Framebuffer;
class Editor;
class CameraNode3d;
class EditorCameraController;

class ViewportWindow {
public:
    ViewportWindow(Editor* editor, std::string  name);
    ~ViewportWindow();

    void Update(float deltaTime);
    void Draw();

    [[nodiscard]] bool IsHovered() const { return m_viewportHovered; }
    [[nodiscard]] CameraNode3d* GetCamera() const { return m_camera.get(); }
    [[nodiscard]] EditorCameraController* GetCameraController() const { return m_cameraController.get(); }

private:
    Editor* m_editor;
    std::string m_name;

    ImVec2 m_viewportPos = {0, 0};
    ImVec2 m_viewportSize = {0, 0};
    bool m_viewportHovered = false;

    float m_timeSinceLastRender = 0.0f;

    std::unique_ptr<CameraNode3d> m_camera;
    std::unique_ptr<EditorCameraController> m_cameraController;
    std::unique_ptr<Framebuffer> m_framebuffer;
};


#endif //MANGORENDERING_VIEWPORTWINDOW_H