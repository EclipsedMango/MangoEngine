
#ifndef MANGORENDERING_CORE_H
#define MANGORENDERING_CORE_H

#include "RenderApi.h"
#include "TreeListener.h"
#include "Nodes/Node3d.h"
#include "Window.h"
#include "Nodes/CameraNode3d.h"
#include "Nodes/RenderableNode3d.h"
#include "Nodes/SkyboxNode3d.h"
#include "Nodes/Lights/LightNode3d.h"

class Core : public TreeListener {
public:
    explicit Core(Node3d* scene);
    ~Core() override;

    Core(const Core&)            = delete;
    Core& operator=(const Core&) = delete;

    void Init();

    void Notification(Node3d* node, NodeNotification notification) override;

    bool PollEvents(); // returns false when window should close
    void RenderScene() const;
    void SwapBuffers() const;
    void StepFrame(float deltaTime);
    void Process();

    void ChangeScene(Node3d* scene);
    void RebuildNodeCache();

    void SetActiveCamera(CameraNode3d* camera);

    static void BeginImGuiFrame();
    static void EndImGuiFrame();

    [[nodiscard]] Node3d* GetScene() const { return m_currentScene; }
    [[nodiscard]] RenderApi& GetRenderer() const { return *m_renderer; }
    [[nodiscard]] Window* GetActiveWindow() const { return m_activeWindow; }

private:
    void InitRenderer();
    void InitImGui() const;

    void BuildNodeCache(Node3d* node);
    [[nodiscard]] bool IsNodeCached(const Node3d* node) const;

    void RegisterLight(LightNode3d* light) const;
    void UnregisterLight(LightNode3d* light) const;

    float m_accumulator = 0.0f;
    static constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

    std::unique_ptr<RenderApi> m_renderer;
    Node3d* m_currentScene = nullptr;
    Window* m_activeWindow = nullptr;

    std::vector<Node3d*> m_nodeCache;
    std::vector<RenderableNode3d*> m_renderableCache;
    SkyboxNode3d* m_activeSkybox = nullptr;

    CameraNode3d* m_activeCamera = nullptr;
    bool m_mouseCaptured = true;
};


#endif //MANGORENDERING_CORE_H