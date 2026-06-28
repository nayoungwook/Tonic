#include "engine/input.h"

#include "engine/display.h"
#include "engine/engine.h"

#include <algorithm>
#include <cmath>

void Input::begin_new_frame() {
	previous_keys = current_keys;
	previous_mouse = current_mouse;

	wheel_x = 0;
	wheel_y = 0;
}

void Input::update_mouse_position(float window_x, float window_y,
	Engine *engine) {
	Display *display = engine->get_display();
	float viewport_x = static_cast<float>(display->get_viewport_x());
	float viewport_y = static_cast<float>(display->get_viewport_y());
	float viewport_width =
		std::max(1.0f, static_cast<float>(display->get_viewport_width()));
	float viewport_height =
		std::max(1.0f, static_cast<float>(display->get_viewport_height()));
	float camera_width = engine->get_camera() == nullptr ?
		viewport_width :
		static_cast<float>(engine->get_camera()->get_width());
	float camera_height = engine->get_camera() == nullptr ?
		viewport_height :
		static_cast<float>(engine->get_camera()->get_height());

	window_mouse_x = window_x;
	window_mouse_y = window_y;
	viewport_mouse_x = std::clamp(window_x - viewport_x, 0.0f,
		viewport_width);
	viewport_mouse_y = std::clamp(window_y - viewport_y, 0.0f,
		viewport_height);

	float normalized_x = viewport_mouse_x / viewport_width;
	float normalized_y = viewport_mouse_y / viewport_height;
	float camera_x = (normalized_x - 0.5f) * camera_width;
	float camera_y = (0.5f - normalized_y) * camera_height;

	mouse_x = static_cast<int>(std::round(camera_x));
	mouse_y = static_cast<int>(std::round(camera_y));
	ui_mouse_x = camera_x;
	ui_mouse_y = camera_y;
}

void Input::process_event(const SDL_Event &event, Engine *engine) {
	switch (event.type) {

		// keyboard
	case SDL_EVENT_KEY_DOWN:
		if (!event.key.repeat)
			current_keys[event.key.scancode] = true;
		break;

	case SDL_EVENT_KEY_UP:
		current_keys[event.key.scancode] = false;
		break;

		// mouse button
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
		update_mouse_position(event.button.x, event.button.y, engine);
		current_mouse[event.button.button] = true;
		break;

	case SDL_EVENT_MOUSE_BUTTON_UP:
		update_mouse_position(event.button.x, event.button.y, engine);
		current_mouse[event.button.button] = false;
		break;

		// mouse motion
	case SDL_EVENT_MOUSE_MOTION: {
		update_mouse_position(event.motion.x, event.motion.y, engine);
		break;
	}

						// mouse wheel
	case SDL_EVENT_MOUSE_WHEEL:
		update_mouse_position(event.wheel.mouse_x, event.wheel.mouse_y,
			engine);
		wheel_x = static_cast<int>(event.wheel.x);
		wheel_y = static_cast<int>(event.wheel.y);
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
float Input::get_window_mouse_x() const { return window_mouse_x; }
float Input::get_window_mouse_y() const { return window_mouse_y; }
float Input::get_viewport_mouse_x() const { return viewport_mouse_x; }
float Input::get_viewport_mouse_y() const { return viewport_mouse_y; }

int Input::get_mouse_wheel_x() const { return wheel_x; }
int Input::get_mouse_wheel_y() const { return wheel_y; }
