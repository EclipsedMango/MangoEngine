
#ifndef MANGORENDERING_INPUT_H
#define MANGORENDERING_INPUT_H

#include <array>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

class Input {
public:
    Input() = delete;

    static void BeginFrame();
    static void ProcessEvent(const SDL_Event& event);
    static void EndFrame();

    static void SetMouseDeltaEnabled(bool enabled);
    static void SetCaptureWindow(SDL_Window* window);
    static void SetMouseCaptureEnabled(bool enabled);
    [[nodiscard]] static bool IsMouseCaptureEnabled() { return m_mouseCaptureEnabled; }

    [[nodiscard]] static bool IsKeyHeld(SDL_Scancode key);
    [[nodiscard]] static bool IsKeyJustPressed(SDL_Scancode key);
    [[nodiscard]] static bool IsKeyJustPressedWithMod(SDL_Scancode key, SDL_Keymod mod);
    [[nodiscard]] static bool IsKeyJustReleased(SDL_Scancode key);

    [[nodiscard]] static bool IsMouseButtonHeld(int button);
    [[nodiscard]] static bool IsMouseButtonJustPressed(int button);
    [[nodiscard]] static bool IsMouseButtonJustReleased(int button);

    [[nodiscard]] static glm::vec2 GetMouseDelta()    { return m_mouseDelta; }
    [[nodiscard]] static glm::vec2 GetMousePosition() { return m_mousePosition; }

    [[nodiscard]] static float GetMouseWheelY() { return m_mouseWheelY; }
    [[nodiscard]] static float GetMouseWheelX() { return m_mouseWheelX; }

private:
    static std::array<bool, SDL_SCANCODE_COUNT> m_current;
    static std::array<bool, SDL_SCANCODE_COUNT> m_previous;

    static glm::vec2 m_mouseDelta;
    static glm::vec2 m_mousePosition;

    static float m_mouseWheelX;
    static float m_mouseWheelY;

    static bool m_mouseDeltaEnabled;
    static bool m_mouseCaptureEnabled;
    static SDL_Window* m_captureWindow;

    static Uint32 m_mouseButtonsCurrent;
    static Uint32 m_mouseButtonsPrevious;
};


#endif //MANGORENDERING_INPUT_H
