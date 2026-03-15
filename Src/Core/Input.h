
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

    [[nodiscard]] static bool IsKeyHeld(SDL_Scancode key);
    [[nodiscard]] static bool IsKeyJustPressed(SDL_Scancode key);
    [[nodiscard]] static bool IsKeyJustReleased(SDL_Scancode key);

    [[nodiscard]] static glm::vec2 GetMouseDelta()    { return m_mouseDelta; }
    [[nodiscard]] static glm::vec2 GetMousePosition() { return m_mousePosition; }

private:
    static std::array<bool, SDL_SCANCODE_COUNT> m_current;
    static std::array<bool, SDL_SCANCODE_COUNT> m_previous;

    static glm::vec2 m_mouseDelta;
    static glm::vec2 m_mousePosition;

    static bool m_mouseDeltaEnabled;
};


#endif //MANGORENDERING_INPUT_H