
#ifndef MANGORENDERING_EDITORSTYLE_H
#define MANGORENDERING_EDITORSTYLE_H
#include "imgui.h"


class EditorStyle {
public:
    static EditorStyle& Get() {
        static EditorStyle instance;
        return instance;
    }

    EditorStyle(const EditorStyle&) = delete;
    EditorStyle& operator=(const EditorStyle&) = delete;

    void Init(ImGuiIO& io) {
        LoadFonts(io);
        ApplyStyle();
    }

    // ----------------------------------------------------------------
    // Fonts
    // ----------------------------------------------------------------
    ImFont* mainFont = nullptr; // Lexend labels, menus, general UI
    ImFont* monoFont = nullptr; // JetBrains Mono values, stats, debug

    // ----------------------------------------------------------------
    // colors
    // ----------------------------------------------------------------
    ImVec4 colHeader = { 0.55f, 0.75f, 0.95f, 1.00f }; // soft blue, section headers, node name, tree nodes
    ImVec4 colLabel  = { 0.75f, 0.75f, 0.75f, 1.00f }; // muted grey, property labels, secondary text
    ImVec4 colSeparator = { 0.30f, 0.30f, 0.35f, 1.00f }; // dark grey, table borders
    ImU32 colTreeLine = IM_COL32(80, 80, 90, 200); // scene tree connector lines

    // ----------------------------------------------------------------
    // scoped helpers, always match Push with Pop
    // ----------------------------------------------------------------

    // used for mono values: FPS, ms, positions, sizes
    void PushMono() const { ImGui::PushFont(monoFont); }
    void PopMono() const { ImGui::PopFont(); }

    // used for panel section headers (SeparatorText, TreeNode labels)
    void PushHeader() const { ImGui::PushStyleColor(ImGuiCol_Text, colHeader); }
    void PopHeader() const { ImGui::PopStyleColor(); }

    // used for secondary / dimmed text
    void PushLabel() const { ImGui::PushStyleColor(ImGuiCol_Text, colLabel); }
    void PopLabel() const { ImGui::PopStyleColor(); }

private:
    EditorStyle() = default;

    void LoadFonts(ImGuiIO& io) {
        // first font loaded becomes the ImGui default
        mainFont = io.Fonts->AddFontFromFileTTF("../Assets/Editor/Fonts/Lexend/Lexend-VariableFont_wght.ttf", 14.0f);
        monoFont = io.Fonts->AddFontFromFileTTF("../Assets/Editor/Fonts/JetBrains_Mono/JetBrainsMono-VariableFont_wght.ttf", 13.0f);
    }

    void ApplyStyle() {
        ImGuiStyle& s = ImGui::GetStyle();

        // ----- rounding -----
        s.WindowRounding = 4.0f;
        s.FrameRounding = 3.0f;
        s.PopupRounding = 3.0f;
        s.ScrollbarRounding = 3.0f;
        s.GrabRounding = 3.0f;
        s.TabRounding = 3.0f;

        // ----- spacing -----
        s.WindowPadding = ImVec2(10.0f, 10.0f);
        s.FramePadding = ImVec2(6.0f,  3.0f);
        s.CellPadding = ImVec2(4.0f,  3.0f);
        s.ItemSpacing = ImVec2(8.0f,  5.0f);
        s.IndentSpacing = 16.0f;
        s.ScrollbarSize = 10.0f;

        // ----- colors -----
        ImVec4* c = s.Colors;

        // base surfaces
        c[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.13f, 0.15f, 1.00f);
        c[ImGuiCol_ChildBg] = ImVec4(0.11f, 0.11f, 0.13f, 1.00f);
        c[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.13f, 0.16f, 1.00f);

        // borders
        c[ImGuiCol_Border] = ImVec4(0.28f, 0.28f, 0.32f, 1.00f);
        c[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        c[ImGuiCol_TableBorderLight] = colSeparator;
        c[ImGuiCol_TableBorderStrong] = ImVec4(0.40f, 0.40f, 0.46f, 1.00f);

        // title bars
        c[ImGuiCol_TitleBg]= ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        c[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.13f, 0.16f, 1.00f);
        c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

        // menu bar
        c[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

        // frames
        c[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.21f, 1.00f);
        c[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
        c[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);

        // buttons
        c[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.24f, 1.00f);
        c[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.28f, 0.34f, 1.00f);
        c[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.35f, 0.42f, 1.00f);

        // headers
        c[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
        c[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.26f, 0.32f, 1.00f);
        c[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.30f, 0.38f, 1.00f);

        // tabs
        c[ImGuiCol_Tab] = ImVec4(0.13f, 0.13f, 0.16f, 1.00f);
        c[ImGuiCol_TabHovered] = ImVec4(0.30f, 0.30f, 0.38f, 1.00f);
        c[ImGuiCol_TabSelected] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
        c[ImGuiCol_TabDimmed] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        c[ImGuiCol_TabDimmedSelected] = ImVec4(0.16f, 0.16f, 0.20f, 1.00f);

        // docking
        c[ImGuiCol_DockingPreview] = ImVec4(0.55f, 0.75f, 0.95f, 0.30f);  // colHeader tint
        c[ImGuiCol_DockingEmptyBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

        // scrollbar
        c[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        c[ImGuiCol_ScrollbarGrab] = ImVec4(0.28f, 0.28f, 0.34f, 1.00f);
        c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.35f, 0.42f, 1.00f);
        c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.42f, 0.42f, 0.50f, 1.00f);

        // separator
        c[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.32f, 1.00f);
        c[ImGuiCol_SeparatorHovered] = colHeader;
        c[ImGuiCol_SeparatorActive] = colHeader;

        // slider / grab
        c[ImGuiCol_SliderGrab] = ImVec4(0.55f, 0.75f, 0.95f, 0.80f);  // colHeader
        c[ImGuiCol_SliderGrabActive] = colHeader;

        // check mark
        c[ImGuiCol_CheckMark] = colHeader;

        // text
        c[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.92f, 1.00f);
        c[ImGuiCol_TextDisabled] = colLabel;
    }
};


#endif //MANGORENDERING_EDITORSTYLE_H