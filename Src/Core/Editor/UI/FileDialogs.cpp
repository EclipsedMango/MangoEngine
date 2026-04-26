#include "FileDialogs.h"

#include <algorithm>

#include "ImGuiFileDialog.h"

namespace {

    constexpr const char* kOpenSceneDialogKey = "MangoOpenSceneDialog";
    constexpr const char* kSaveSceneDialogKey = "MangoSaveSceneDialog";

    constexpr const char* kOpenSceneTitle = "Open Scene";
    constexpr const char* kSaveSceneTitle = "Save Scene";

    constexpr const char* kSceneFilter = ".mscn";

    enum class ActiveDialog {
        None,
        OpenScene,
        SaveScene
    };

    ActiveDialog g_activeDialog = ActiveDialog::None;
    FileDialogs::PathCallback g_onAccepted;

    std::string ToLower(std::string s) {
        std::ranges::transform(s, s.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        return s;
    }

    std::filesystem::path EnsureMscnExtension(std::filesystem::path path) {
        if (ToLower(path.extension().string()) != ".mscn") {
            path.replace_extension(".mscn");
        }

        return path;
    }

    std::string GetDialogStartPath(const std::filesystem::path& suggestedPath) {
        namespace fs = std::filesystem;

        std::error_code ec;

        fs::path startPath;

        if (!suggestedPath.empty() && suggestedPath.has_parent_path()) {
            startPath = suggestedPath.parent_path();
        } else {
            startPath = fs::current_path(ec);
        }

        if (startPath.empty()) {
            startPath = ".";
        }

        if (!fs::exists(startPath, ec)) {
            fs::create_directories(startPath, ec);
        }

        if (ec || !fs::exists(startPath) || !fs::is_directory(startPath)) {
            startPath = fs::current_path(ec);
        }

        if (startPath.empty()) {
            startPath = ".";
        }

        return startPath.string();
    }

    std::string GetSuggestedFileName(const std::filesystem::path& suggestedPath) {
        if (!suggestedPath.empty() && suggestedPath.has_filename()) {
            return suggestedPath.filename().string();
        }

        return "Untitled.mscn";
    }

    void ResetDialogState() {
        g_activeDialog = ActiveDialog::None;
        g_onAccepted = {};
    }

}

namespace FileDialogs {

    void OpenSceneDialog(PathCallback onAccepted) {
        if (g_activeDialog != ActiveDialog::None) {
            return;
        }

        g_activeDialog = ActiveDialog::OpenScene;
        g_onAccepted = std::move(onAccepted);

        IGFD::FileDialogConfig config;
        config.path = std::filesystem::current_path().string();
        config.countSelectionMax = 1;

        ImGuiFileDialog::Instance()->OpenDialog(
            kOpenSceneDialogKey,
            kOpenSceneTitle,
            kSceneFilter,
            config
        );
    }

    void SaveSceneDialog(const std::filesystem::path& suggestedPath, PathCallback onAccepted) {
        if (g_activeDialog != ActiveDialog::None) {
            return;
        }

        g_activeDialog = ActiveDialog::SaveScene;
        g_onAccepted = std::move(onAccepted);

        IGFD::FileDialogConfig config;
        config.path = GetDialogStartPath(suggestedPath);
        config.fileName = GetSuggestedFileName(suggestedPath);
        config.countSelectionMax = 1;
        config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

        ImGuiFileDialog::Instance()->OpenDialog(kSaveSceneDialogKey, kSaveSceneTitle, kSceneFilter, config);
    }

    void Draw() {
        if (g_activeDialog == ActiveDialog::None) {
            return;
        }

        ImGuiFileDialog* dialog = ImGuiFileDialog::Instance();

        const char* key = nullptr;

        switch (g_activeDialog) {
            case ActiveDialog::OpenScene: {
                key = kOpenSceneDialogKey;
                break;
            }
            case ActiveDialog::SaveScene: {
                key = kSaveSceneDialogKey;
                break;
            }
            case ActiveDialog::None:
            default: {
                return;
            }
        }

        if (dialog->Display(key)) {
            if (dialog->IsOk()) {
                std::filesystem::path selectedPath = dialog->GetFilePathName();

                if (g_activeDialog == ActiveDialog::SaveScene) {
                    selectedPath = EnsureMscnExtension(selectedPath);
                }

                if (g_onAccepted) {
                    g_onAccepted(selectedPath);
                }
            }

            dialog->Close();
            ResetDialogState();
        }
    }

    bool IsOpen() {
        return g_activeDialog != ActiveDialog::None;
    }

}