
#include "Input.h"

std::array<bool, SDL_SCANCODE_COUNT> Input::m_current = {};
std::array<bool, SDL_SCANCODE_COUNT> Input::m_previous = {};

glm::vec2 Input::m_mouseDelta = {};
glm::vec2 Input::m_mousePosition = {};

bool Input::m_mouseDeltaEnabled = true;

void Input::BeginFrame() {
    m_previous = m_current;
    m_mouseDelta = {};
}

void Input::ProcessEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_MOUSE_MOTION: {
            m_mousePosition = {
                static_cast<float>(event.motion.x),
                static_cast<float>(event.motion.y)
            };

            if (m_mouseDeltaEnabled) {
                m_mouseDelta.x += static_cast<float>(event.motion.xrel);
                m_mouseDelta.y += static_cast<float>(event.motion.yrel);
            }

            break;
        }

        default: {
            break;
        }
    }
}

void Input::EndFrame() {
    int numKeys = 0;
    const bool* sdlKeys = SDL_GetKeyboardState(&numKeys);

    for (int i = 0; i < numKeys && i < SDL_SCANCODE_COUNT; ++i) {
        m_current[i] = sdlKeys[i];
    }

    float mx = 0.0f;
    float my = 0.0f;
    SDL_GetMouseState(&mx, &my);
    m_mousePosition = { mx, my };
}

void Input::SetMouseDeltaEnabled(const bool enabled) {
    m_mouseDeltaEnabled = enabled;
    if (!enabled) {
        m_mouseDelta = {};
    }
}

bool Input::IsKeyHeld(const SDL_Scancode key) {
    return m_current[key];
}

bool Input::IsKeyJustPressed(const SDL_Scancode key) {
    return m_current[key] && !m_previous[key];
}

bool Input::IsKeyJustReleased(const SDL_Scancode key) {
    return !m_current[key] && m_previous[key];
}
