#include "engine/input.h"

#include "engine/display.h"
#include "engine/engine.h"

void Input::begin_new_frame() {
        previous_keys = current_keys;
        previous_mouse = current_mouse;

        wheel_x = 0;
        wheel_y = 0;
}

void Input::process_event(const SDL_Event &event, Engine *engine) {
        switch (event.type) {

                // keyboard
        case SDL_KEYDOWN:
                if (!event.key.repeat)
                        current_keys[event.key.keysym.scancode] = true;
                break;

        case SDL_KEYUP:
                current_keys[event.key.keysym.scancode] = false;
                break;

                // mouse button
        case SDL_MOUSEBUTTONDOWN:
                current_mouse[event.button.button] = true;
                break;

        case SDL_MOUSEBUTTONUP:
                current_mouse[event.button.button] = false;
                break;

                // mouse motion
        case SDL_MOUSEMOTION: {
                float w_offset = engine->get_display()->get_width() / 2;
                float h_offset = engine->get_display()->get_height() / 2;
                mouse_x = event.motion.x - w_offset;
                mouse_y = (event.motion.y - h_offset) * -1;

                Display *display = engine->get_display();
                float vx = (float)display->get_viewport_x();
                float vy = (float)display->get_viewport_y();
                float vw = (float)display->get_viewport_width();
                float vh = (float)display->get_viewport_height();
                ui_mouse_x = vw > 0.0f ? event.motion.x - vx - vw * 0.5f : 0.0f;
                ui_mouse_y = vh > 0.0f
                                  ? (event.motion.y - vy - vh * 0.5f) * -1.0f
                                  : 0.0f;
                break;
        }

        // mouse wheel
        case SDL_MOUSEWHEEL:
                wheel_x += event.wheel.x;
                wheel_y += event.wheel.y;
                break;
        }
}

bool Input::is_key_down(SDL_Scancode key) const {
        auto it = current_keys.find(key);
        return it != current_keys.end() && it->second;
}

bool Input::is_key_pressed(SDL_Scancode key) const {
        bool current = is_key_down(key);
        bool previous =
            previous_keys.count(key) ? previous_keys.at(key) : false;
        return current && !previous;
}

bool Input::is_key_released(SDL_Scancode key) const {
        bool current = is_key_down(key);
        bool previous =
            previous_keys.count(key) ? previous_keys.at(key) : false;
        return !current && previous;
}

bool Input::is_mouse_down(Uint8 button) const {
        auto it = current_mouse.find(button);
        return it != current_mouse.end() && it->second;
}

bool Input::is_mouse_pressed(Uint8 button) const {
        bool current = is_mouse_down(button);
        bool previous =
            previous_mouse.count(button) ? previous_mouse.at(button) : false;
        return current && !previous;
}

bool Input::is_mouse_released(Uint8 button) const {
        bool current = is_mouse_down(button);
        bool previous =
            previous_mouse.count(button) ? previous_mouse.at(button) : false;
        return !current && previous;
}

int Input::get_mouse_x() const { return mouse_x; }
int Input::get_mouse_y() const { return mouse_y; }
float Input::get_ui_mouse_x() const { return ui_mouse_x; }
float Input::get_ui_mouse_y() const { return ui_mouse_y; }

int Input::get_mouse_wheel_x() const { return wheel_x; }
int Input::get_mouse_wheel_y() const { return wheel_y; }
