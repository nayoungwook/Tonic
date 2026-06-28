#include "engine/display.h"

#include "engine/camera.h"
#include "engine/error.h"
#include "engine/framebuffer.h"

#include <algorithm>
#include <cmath>

Display::Display(const std::string &title, int width, int height)
	: title(title), width(width), height(height) {
	window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, width, height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if (window == nullptr) {
		error("Window creation failed.");
	}

	gl_context = SDL_GL_CreateContext(window);
	if (!gl_context) {
		printf("Failed to create GL context: %s\n", SDL_GetError());
	}

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		error("GLEW Init Failed");
	}

	update_viewport(width, height);
}

Display::~Display() {
	SDL_DestroyWindow(window);
	SDL_Quit();
}

SDL_Window *Display::get_window() { return window; }

void Display::update_viewport(int win_width, int win_height) {
	width = std::max(1, win_width);
	height = std::max(1, win_height);

	if (camera == nullptr) {
		viewport_width = width;
		viewport_height = height;
		viewport_x = 0;
		viewport_y = 0;

		goto update_viewport;
	}

	bool pixel_perfect_enabled = pixel_perfect_screen && camera != nullptr &&
		camera->is_pixel_perfect_enabled();

	if (pixel_perfect_enabled) {
		int render_width = camera->get_pixel_view_width();
		int render_height = camera->get_pixel_view_height();
		int scale = std::max(1,
			std::min(width / render_width, height / render_height));

		viewport_width = render_width * scale;
		viewport_height = render_height * scale;
		viewport_x = (width - viewport_width) / 2;
		viewport_y = (height - viewport_height) / 2;
	}
	else {
		float target_aspect =
			static_cast<float>(camera->get_width()) / camera->get_height();
		float window_aspect = static_cast<float>(width) / height;

		if (window_aspect > target_aspect) {
			viewport_width = static_cast<int>(height * target_aspect);
			viewport_height = height;
			viewport_x = (width - viewport_width) / 2;
			viewport_y = 0;
		}
		else {
			viewport_width = width;
			viewport_height = static_cast<int>(width / target_aspect);
			viewport_x = 0;
			viewport_y = (height - viewport_height) / 2;
		}
	}

update_viewport:
	glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
	FrameBuffer::invalidate_bind_cache();
	FrameBuffer::resize_camera_sized_framebuffers();
}

void Display::apply_screen_viewport() {
	glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
	FrameBuffer::invalidate_bind_cache();
}

void Display::set_pixel_perfect_screen(bool enabled) {
	pixel_perfect_screen = enabled;
	if (camera != nullptr)
		camera->set_pixel_perfect_enabled(enabled);
	update_viewport(width, height);
}

bool Display::is_pixel_perfect_screen() const {
	return pixel_perfect_screen;
}

void Display::configure_pixel_perfect(int reference_width,
	int reference_height, int assets_pixels_per_unit) {
	if (camera != nullptr) {
		camera->configure_pixel_perfect(static_cast<float>(reference_width),
			static_cast<float>(reference_height),
			static_cast<float>(assets_pixels_per_unit));
	}
	pixel_perfect_screen = true;
	update_viewport(width, height);
}

void Display::configure_pixel_perfect_for_sprite(int sprite_pixel_size,
	float world_size) {
	if (sprite_pixel_size <= 0 || world_size <= 0.0f)
		return;

	if (camera != nullptr) {
		float world_units_per_pixel = world_size / sprite_pixel_size;
		float minimum_render_width =
			camera->get_base_width() / world_units_per_pixel;
		float minimum_render_height =
			camera->get_base_height() / world_units_per_pixel;
		int integer_scale = std::max(1, static_cast<int>(std::floor(
			std::min(width / minimum_render_width,
				height / minimum_render_height))));
		int reference_width = std::max(1, width / integer_scale);
		int reference_height = std::max(1, height / integer_scale);

		camera->configure_pixel_perfect(
			static_cast<float>(reference_width),
			static_cast<float>(reference_height),
			static_cast<float>(sprite_pixel_size) / world_size);
	}
	pixel_perfect_screen = true;
	update_viewport(width, height);
}

void Display::set_pixel_snapping(bool enabled) {
	if (camera != nullptr)
		camera->set_pixel_perfect_enabled(enabled);
}

void Display::set_fullscreen() {
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	int w = 0;
	int h = 0;
	SDL_GetWindowSize(window, &w, &h);
	update_viewport(w, h);
}

void Display::set_windowed(int width, int height) {
	SDL_SetWindowFullscreen(window, 0);
	SDL_SetWindowSize(window, std::max(1, width), std::max(1, height));
	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED);
	update_viewport(std::max(1, width), std::max(1, height));
}

std::string Display::get_title() { return title; }

int Display::get_width() { return width; }

int Display::get_height() { return height; }

void Display::set_camera(Camera *camera) { this->camera = camera; }

int Display::get_viewport_x() const { return viewport_x; }

int Display::get_viewport_y() const { return viewport_y; }

int Display::get_viewport_width() const { return viewport_width; }

int Display::get_viewport_height() const { return viewport_height; }
