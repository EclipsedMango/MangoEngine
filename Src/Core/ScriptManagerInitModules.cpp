
#include "ScriptManager.h"
#include "ScriptManagerInternal.h"

#include "Input.h"

void RegisterInputModule(sol::state& lua) {
    sol::table inputModule = lua.create_named_table("Input");
    inputModule.set_function("IsKeyPressed", &Input::IsKeyJustPressed);
    inputModule.set_function("IsKeyHeld", &Input::IsKeyHeld);
    inputModule.set_function("IsMouseButtonPressed", &Input::IsMouseButtonJustPressed);
    inputModule.set_function("GetMouseDelta", &Input::GetMouseDelta);
    inputModule.set_function("SetMouseDeltaEnabled", &Input::SetMouseDeltaEnabled);
    inputModule.set_function("SetMouseCaptureEnabled", &Input::SetMouseCaptureEnabled);
    inputModule.set_function("IsMouseCaptureEnabled", &Input::IsMouseCaptureEnabled);

    sol::table keys = lua.create_table();
    keys["W"] = SDL_SCANCODE_W;
    keys["A"] = SDL_SCANCODE_A;
    keys["S"] = SDL_SCANCODE_S;
    keys["D"] = SDL_SCANCODE_D;
    keys["Q"] = SDL_SCANCODE_Q;
    keys["E"] = SDL_SCANCODE_E;
    keys["SPACE"] = SDL_SCANCODE_SPACE;
    keys["LCTRL"] = SDL_SCANCODE_LCTRL;
    keys["LSHIFT"] = SDL_SCANCODE_LSHIFT;
    keys["ESCAPE"] = SDL_SCANCODE_ESCAPE;
    keys["TAB"] = SDL_SCANCODE_TAB;
    inputModule["Key"] = keys;
}

void RegisterAppModule(sol::state& lua, ScriptManager* mgr) {
    sol::table appModule = lua.create_named_table("App");
    appModule.set_function("Quit", [mgr] { mgr->RequestQuit(); });
}