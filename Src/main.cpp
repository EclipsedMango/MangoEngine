#include "Core/Core.h"
#include "Core/ResourceManager.h"
#include "Core/Editor/Editor.h"
#include "Nodes/MeshNode3d.h"
#include "Nodes/PortalNode3d.h"
#include "Nodes/Lights/DirectionalLightNode3d.h"
#include "Nodes/Lights/PointLightNode3d.h"
#include "Renderer/Meshes/GltfLoader.h"
#include "Renderer/Meshes/PrimitiveMesh.h"

int main() {
    auto scene = std::make_unique<Node3d>();
    Editor editor(std::move(scene));

    Node3d* liveScene = editor.GetActiveViewport()->GetScene();

    const auto cubeMesh = std::make_shared<CubeMesh>();
    const auto sphereMesh = std::make_shared<SphereMesh>();

    const auto portalShader = ResourceManager::Get().LoadShader("portal_mask", "portal_mask.vert", "portal_mask.frag");

    auto portalA = std::make_unique<PortalNode3d>();
    portalA->SetMeshByName("Quad");
    portalA->SetName("Portal1");
    portalA->SetPosition({0, 0, 0.1f});
    portalA->SetScale({1.5f, 2.5f, 1.0f});
    portalA->GetActiveMaterial()->SetShader(portalShader);
    PortalNode3d* portalAPtr = portalA.get();
    liveScene->AddChild(std::move(portalA));

    auto portalB = std::make_unique<PortalNode3d>();
    portalB->SetMeshByName("Quad");
    portalB->SetName("Portal2");
    portalB->SetPosition({3.5f, 6.25f, -5.5f});
    portalB->SetRotationEuler({180, -30, 180});
    portalB->SetScale({1.5f, 2.5f, 1.0f});
    portalB->GetActiveMaterial()->SetShader(portalShader);
    PortalNode3d* portalBPtr = portalB.get();
    liveScene->AddChild(std::move(portalB));

    PortalNode3d::LinkPair(portalAPtr, portalBPtr);

    auto sun = std::make_unique<DirectionalLightNode3d>(glm::vec3(0.5f, -0.6f, -0.5f), glm::vec3(0.9f, 0.65f, 0.32f), 2.5f);
    liveScene->AddChild(std::move(sun));

    auto skybox = std::make_unique<SkyboxNode3d>(ResourceManager::Get().ResolveAssetPath("kloppenheim_06_puresky_1k.hdr"));
    editor.GetCore().SetGlobalSkybox(std::move(skybox));

    auto camera = std::make_unique<CameraNode3d>(glm::vec3(0.0f, 1.0f, 8.0f), 75.0f, 1280.0f / 720.0f);
    camera->SetScript("Root/hello.lua");
    camera->SetAsGameCamera(true);
    liveScene->AddChild(std::move(camera));

    editor.Run();

    scene.reset();

    return 0;
}
