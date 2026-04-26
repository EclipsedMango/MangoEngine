
#ifndef MANGORENDERING_FILEDIALOGS_H
#define MANGORENDERING_FILEDIALOGS_H

#include <filesystem>
#include <functional>
#include <optional>

namespace FileDialogs {
    using PathCallback = std::function<void(const std::filesystem::path&)>;

    void OpenSceneDialog(PathCallback onAccepted);
    void SaveSceneDialog(const std::filesystem::path& suggestedPath, PathCallback onAccepted);

    // called this once per ImGui frame
    void Draw();

    bool IsOpen();
}

#endif //MANGORENDERING_FILEDIALOGS_H
