
#include "ContentBrowserWindow.h"

#include <fstream>

#include "imgui.h"
#include "Core/Editor/EditorStyle.h"

ContentBrowserWindow::ContentBrowserWindow(Editor *editor) : m_editor(editor), m_currentPath(fs::current_path() / "Root") {}

void ContentBrowserWindow::DrawContentBrowser() {
    ImGui::Begin("Content Browser");

    const auto& style = EditorStyle::Get();

    style.PushLabel();
    ImGui::Text("Current Directory: %s", m_currentPath.string().c_str());
    EditorStyle::PopLabel();

    if (ImGui::Button("Back", ImVec2(64, 32))) {
        if (m_currentPath.filename() != "Root") {
            m_currentPath = m_currentPath.parent_path();
        }
    }

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::GetDragDropPayload() != nullptr) {
        if (m_backHoverStart == 0.0) {
            m_backHoverStart = ImGui::GetTime();
        } else if (ImGui::GetTime() - m_backHoverStart >= m_backHoverDelay) {
            if (m_currentPath.filename() != "Root") {
                m_currentPath = m_currentPath.parent_path();
            }
            m_backHoverStart = 0.0;
        }
    } else {
        m_backHoverStart = 0.0;
    }

    ImGui::Separator();
    DisplayPath(m_currentPath);

    if (m_backHoverStart == 0.0) {
        ImGui::SetNextItemAllowOverlap();
        ImGui::SetCursorScreenPos(ImGui::GetWindowPos());
        ImGui::InvisibleButton("##window_drop_target", ImGui::GetWindowSize());
        AcceptDrop(m_currentPath);

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup("##bg_context");
        }
    }

    if (ImGui::BeginPopup("##bg_context")) {
        if (ImGui::MenuItem("New Folder")) NewFolder(m_currentPath);
        if (ImGui::MenuItem("New File")) NewFile(m_currentPath);
        ImGui::EndPopup();
    }

    ImGui::End();
}

void ContentBrowserWindow::DisplayPath(const fs::path &path) {
    constexpr float itemSize = 80.0f;
    constexpr float padding = 10.0f;
    const float windowWidth = ImGui::GetContentRegionAvail().x;
    const int columns = std::max(1, static_cast<int>(windowWidth / (itemSize + padding)));

    int i = 0;
    for (const auto& entry : fs::directory_iterator(path)) {
        std::string filename = entry.path().filename().string();
        std::string fullPath = entry.path().string();

        if (i % columns != 0) {
            ImGui::SameLine(0, padding);
        }

        ImGui::BeginGroup();

        ImVec2 startPos = ImGui::GetCursorScreenPos();

        ImGui::PushID(filename.c_str());
        const bool clicked = ImGui::InvisibleButton("##item_hitbox", ImVec2(itemSize, itemSize));
        const bool rightClick = ImGui::IsItemClicked(ImGuiMouseButton_Right);
        const bool hovered = ImGui::IsItemHovered();
        ImGui::PopID();

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        if (hovered) {
            drawList->AddRectFilled(startPos, ImVec2(startPos.x + itemSize, startPos.y + itemSize), IM_COL32(255, 255, 255, 20), 4.0f);
        }

        const char* icon;
        ImU32 iconColor = IM_COL32_WHITE;
        ImVec2 iconOffset = ImVec2(0, 0);

        if (entry.is_directory()) {
            if (clicked) {
                m_currentPath = entry.path();
            }

            if (rightClick) {
                ImGui::OpenPopup("##fol_context");
                m_contextPayloadPath = relative(entry.path());
            }

            icon = "\uf114";
            iconColor = IM_COL32(180, 180, 180, 255);
            iconOffset = ImVec2(-7.5, 0);

            AcceptDrop(entry.path());
            StartDrag(fullPath, filename);
        } else {
            if (clicked) {
                // TODO: handle file selection
            }

            icon = "\uf016";
            iconColor = IM_COL32(180, 180, 180, 255);
            iconOffset = ImVec2(-2.5, 0);

            StartDrag(fullPath, filename);
        }

        ImGui::PushFont(EditorStyle::Get().mainFontExLg);
        const ImVec2 iconSize = ImGui::CalcTextSize(icon);
        drawList->AddText(ImVec2(startPos.x + (itemSize - iconSize.x) * 0.5f + iconOffset.x, startPos.y + iconOffset.y), iconColor, icon);
        ImGui::PopFont();

        const ImVec2 textSize = ImGui::CalcTextSize(filename.c_str());
        float textX = startPos.x + (itemSize - textSize.x) * 0.5f;

        if (textSize.x > itemSize) textX = startPos.x;

        drawList->PushClipRect(startPos, ImVec2(startPos.x + itemSize, startPos.y + itemSize), true);
        drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(textX, startPos.y + itemSize - textSize.y - 2.0f), IM_COL32(230, 230, 230, 255), filename.c_str());
        drawList->PopClipRect();

        ImGui::EndGroup();
        i++;
    }

    if (ImGui::BeginPopup("##fol_context")) {
        if (ImGui::MenuItem("Open Folder")) {
            m_currentPath = m_contextPayloadPath;
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Copy Path")) {
            const std::string pathStr = m_contextPayloadPath.string();
            ImGui::SetClipboardText(pathStr.c_str());
        }
        if (ImGui::MenuItem("Copy Absolute Path")) {
            const std::string absPathStr = fs::absolute(m_contextPayloadPath).string();
            ImGui::SetClipboardText(absPathStr.c_str());
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Duplicate Folder")) {}
        if (ImGui::MenuItem("Rename Folder")) {}
        if (ImGui::MenuItem("Delete Folder")) {}

        ImGui::Separator();

        if (ImGui::MenuItem("Open in Terminal")) {}
        if (ImGui::MenuItem("Open in File Manager")) {}

        ImGui::EndPopup();
    }
}

void ContentBrowserWindow::NewFolder(const fs::path &path) {
    const std::string baseName = "NewFolder";
    fs::path newFolderPath = path / baseName;

    int counter = 1;
    while (fs::exists(newFolderPath)) {
        newFolderPath = path / (baseName + std::to_string(counter));
        counter++;
    }

    try {
        fs::create_directory(newFolderPath);
    } catch (const fs::filesystem_error& e) {
        // TODO: log to console or something idk
        throw e;
    }
}

void ContentBrowserWindow::NewFile(fs::path &path) {
    const std::string baseName = "NewFile";
    const std::string extension = ".txt";

    fs::path newFilePath = path / (baseName + extension);

    int counter = 1;
    while (fs::exists(newFilePath)) {
        newFilePath = path / (baseName + std::to_string(counter) + extension);
        counter++;
    }

    try {
        std::ofstream newFile(newFilePath);

        if (!newFile.is_open()) {
            throw std::runtime_error("Failed to create file at: " + newFilePath.string());
        }

        newFile.close();
    } catch (const std::exception& e) {
        // TODO: log to console or something idk
        throw e;
    }
}

void ContentBrowserWindow::StartDrag(const std::string &fullPath, const std::string &filename) {
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        ImGui::SetDragDropPayload("FILE_PATH", fullPath.c_str(), fullPath.size() + 1);
        ImGui::Text("Dragging: %s", filename.c_str());
        ImGui::EndDragDropSource();
    }
}

void ContentBrowserWindow::AcceptDrop(const fs::path &destDir) {
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH")) {
            const fs::path sourcePath = static_cast<const char*>(payload->Data);
            try {
                fs::rename(sourcePath, destDir / sourcePath.filename());
            } catch (const fs::filesystem_error& e) {}
        }
        ImGui::EndDragDropTarget();
    }
}
