
#ifndef MANGORENDERING_CONTENTBROWSERWINDOW_H
#define MANGORENDERING_CONTENTBROWSERWINDOW_H

#include <filesystem>

class Editor;

namespace fs = std::filesystem;

class ContentBrowserWindow {
public:
    explicit ContentBrowserWindow(Editor* editor);
    ~ContentBrowserWindow() = default;

    void DrawContentBrowser();

private:
    void DisplayPath(const fs::path& path);

    static void StartDrag(const std::string& fullPath, const std::string& filename);
    static void AcceptDrop(const fs::path& destDir);

    Editor* m_editor = nullptr;
    fs::path m_currentPath;

    double m_backHoverStart = 0.0;
    const double m_backHoverDelay = 0.4;
};


#endif //MANGORENDERING_CONTENTBROWSERWINDOW_H