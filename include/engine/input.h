#pragma once
#include <SDL3/SDL.h>
#include <unordered_map>

class Engine;

class Input {
public:
    void begin_new_frame();
    void process_event(const SDL_Event &event, Engine *engine);

    // keyboard
    bool is_key_down(SDL_Scancode key) const;
    bool is_key_pressed(SDL_Scancode key) const;
    bool is_key_released(SDL_Scancode key) const;

    // mouse buttons
    bool is_mouse_down(Uint8 button) const;
    bool is_mouse_pressed(Uint8 button) const;
    bool is_mouse_released(Uint8 button) const;

    // mouse position
    int get_mouse_x() const;
    int get_mouse_y() const;
    float get_ui_mouse_x() const;
    float get_ui_mouse_y() const;
    float get_window_mouse_x() const;
    float get_window_mouse_y() const;
    float get_viewport_mouse_x() const;
    float get_viewport_mouse_y() const;

    // mouse wheel
    int get_mouse_wheel_x() const;
    int get_mouse_wheel_y() const;

private:
    std::unordered_map<SDL_Scancode, bool> current_keys;
    std::unordered_map<SDL_Scancode, bool> previous_keys;

    std::unordered_map<Uint8, bool> current_mouse;
    std::unordered_map<Uint8, bool> previous_mouse;

    void update_mouse_position(float window_x, float window_y, Engine *engine);

    int mouse_x = 0;
    int mouse_y = 0;
    float ui_mouse_x = 0.0f;
    float ui_mouse_y = 0.0f;
    float window_mouse_x = 0.0f;
    float window_mouse_y = 0.0f;
    float viewport_mouse_x = 0.0f;
    float viewport_mouse_y = 0.0f;

    int wheel_x = 0;
    int wheel_y = 0;
};
