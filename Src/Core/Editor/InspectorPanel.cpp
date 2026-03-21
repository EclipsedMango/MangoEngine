
#include "InspectorPanel.h"

#include <iostream>
#include <memory>
#include <ostream>
#include <ranges>

#include "Editor.h"
#include "imgui.h"
#include "glm/glm.hpp"
#include "Nodes/MeshNode3d.h"
#include "Nodes/Node3d.h"
#include "Nodes/Lights/DirectionalLightNode3d.h"
#include "Nodes/Lights/PointLightNode3d.h"
#include "Renderer/Meshes/PrimitiveMesh.h"

InspectorPanel::InspectorPanel(Editor* editor) : m_editor(editor) {}

void InspectorPanel::DrawInspector(Node3d* selectedNode) {
    ImGui::Begin("Inspector");

    if (!selectedNode) {
        ImGui::TextDisabled("No node selected");
        ImGui::End();
        return;
    }

    // name field
    char nameBuf[256];
    strncpy(nameBuf, selectedNode->GetName().c_str(), sizeof(nameBuf));
    nameBuf[sizeof(nameBuf) - 1] = '\0';
    if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
        selectedNode->SetName(nameBuf);
    }

    ImGui::Separator();

    DrawProperties(selectedNode);

    ImGui::End();
}

void InspectorPanel::DrawProperties(PropertyHolder* holder) {
    for (const auto& name : holder->GetPropertyOrder()) {
        DrawPropertyValue(name, holder);
    }
}

void InspectorPanel::DrawPropertyValue(const std::string& name, PropertyHolder* holder) {
    PropertyValue value = holder->GetProperty(name);

    ImGui::PushID(name.c_str());

    std::visit([&]<typename T0>(T0&& val) {
        using T = std::decay_t<T0>;

        if constexpr (std::is_same_v<T, float>) {
            float v = val;
            if (ImGui::DragFloat(name.c_str(), &v, 0.1f)) {
                holder->Set(name, v);
            }
        }

        if constexpr (std::is_same_v<T, int>) {
            int v = val;
            if (ImGui::DragInt(name.c_str(), &v, 1)) {
                holder->Set(name, v);
            }
        }

        if constexpr (std::is_same_v<T, bool>) {
            bool v = val;
            if (ImGui::Checkbox(name.c_str(), &v)) {
                holder->Set(name, v);
            }
        }

        if constexpr (std::is_same_v<T, std::string>) {
            char buf[256];
            strncpy(buf, val.c_str(), sizeof(buf));
            buf[sizeof(buf) - 1] = '\0';
            if (ImGui::InputText(name.c_str(), buf, sizeof(buf))) {
                holder->Set(name, std::string(buf));
            }
        }

        if constexpr (std::is_same_v<T, glm::vec2>) {
            glm::vec2 v = val;
            if (ImGui::DragFloat2(name.c_str(), &v.x, 0.1f)) {
                holder->Set(name, v);
            }
        }

        if constexpr (std::is_same_v<T, glm::vec3>) {
            glm::vec3 v = val;
            if (ImGui::DragFloat3(name.c_str(), &v.x, 0.1f)) {
                holder->Set(name, v);
            }
        }

        if constexpr (std::is_same_v<T, std::shared_ptr<PropertyHolder>>) {
            if (val && ImGui::TreeNode(name.c_str())) {
                DrawProperties(val.get());
                ImGui::TreePop();
            }
        }
    }, value);

    ImGui::PopID();
}

void InspectorPanel::DrawTexturePreviewPopup() {
    if (!m_previewTex) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(520, 560), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    bool open = true;
    if (ImGui::Begin(m_previewLabel.c_str(), &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize)) {

        const float panelW = ImGui::GetContentRegionAvail().x;

        static int  channel = 0; // 0=RGB, 1=R, 2=G, 3=B, 4=A
        static bool flipY = false;
        static float zoom = 1.0f;

        ImGui::SetNextItemWidth(160);
        ImGui::Combo("Channel##prev", &channel, "RGB\0Red\0Green\0Blue\0Alpha\0");
        ImGui::SameLine();
        ImGui::Checkbox("Flip Y", &flipY);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        ImGui::SliderFloat("Zoom", &zoom, 0.25f, 4.0f, "%.2fx");
        ImGui::SameLine();
        if (ImGui::SmallButton("Reset")) {
            zoom = 1.0f;
            channel = 0;
            flipY = false;
        }

        ImGui::Separator();

        const float imgSize = panelW * zoom;
        const ImVec2 imgSizeV = { imgSize, imgSize };

        const ImVec2 uv0 = flipY ? ImVec2(0, 1) : ImVec2(0, 0);
        const ImVec2 uv1 = flipY ? ImVec2(1, 0) : ImVec2(1, 1);

        // tint per channel selection
        ImVec4 tint = { 1, 1, 1, 1 };
        if (channel == 1) tint = { 1, 0, 0, 1 };
        else if (channel == 2) tint = { 0, 1, 0, 1 };
        else if (channel == 3) tint = { 0, 0, 1, 1 };
        else if (channel == 4) tint = { 1, 1, 1, 1 }; // alpha shown as grey via separate pass

        ImGui::BeginChild("##previewScroll", ImVec2(panelW, panelW), false, ImGuiWindowFlags_HorizontalScrollbar);

        const ImVec2 imgPos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        constexpr float checkSize = 12.0f;
        for (float cy = 0; cy < imgSize; cy += checkSize) {
            for (float cx = 0; cx < imgSize; cx += checkSize) {
                const bool even = (static_cast<int>(cx / checkSize) + static_cast<int>(cy / checkSize)) % 2 == 0;
                dl->AddRectFilled(
                    { imgPos.x + cx, imgPos.y + cy },
                    { imgPos.x + cx + checkSize, imgPos.y + cy + checkSize },
                    even ? IM_COL32(80, 80, 80, 255) : IM_COL32(50, 50, 50, 255)
                );
            }
        }

        ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(m_previewTex->GetGLHandle())), imgSizeV, uv0, uv1, tint, ImVec4(0, 0, 0, 0));

        ImGui::EndChild();

        ImGui::Separator();
        ImGui::TextDisabled("%dx%d  |  %d channels  |  GL handle: %u",
            m_previewTex->GetWidth(),
            m_previewTex->GetHeight(),
            m_previewTex->GetChannels(),
            m_previewTex->GetGLHandle()
        );
    }

    ImGui::End();

    if (!open) {
        m_previewTex = nullptr;
    }
}

void InspectorPanel::OpenTexturePreview(const Texture *tex, const char *label) {
    m_previewTex = tex;
    m_previewLabel = std::string("Texture: ") + label + "###TexPreview";
}

void InspectorPanel::SectionHeader(const char* label) {
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 180, 180, 255));
    ImGui::SeparatorText(label);
    ImGui::PopStyleColor();
}

void InspectorPanel::TextureSlot(const char* label, const std::shared_ptr<Texture>& tex) {
    const bool has = tex != nullptr;

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted(label);
    ImGui::TableSetColumnIndex(1);

    if (has) {
        ImGui::PushID(label);
        const bool clicked = ImGui::ImageButton( "##thumb", static_cast<ImTextureID>(static_cast<intptr_t>(tex->GetGLHandle())), ImVec2(32, 32));
        ImGui::PopID();

        if (clicked) {
            OpenTexturePreview(tex.get(), label);
        }

        ImGui::SameLine();
        ImGui::TextDisabled("%dx%d", tex->GetWidth(), tex->GetHeight());
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(50, 50, 50, 200));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(60, 60, 60, 200));
        ImGui::PushID(label);
        ImGui::Button("##empty", ImVec2(32, 32));
        ImGui::PopID();
        ImGui::PopStyleColor(2);
        ImGui::SameLine();
        ImGui::TextDisabled("None");
    }
}
