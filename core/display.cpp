#include "engine/display.h"

#include "engine/camera.h"
#include "engine/error.h"
#include "engine/framebuffer.h"

#include <algorithm>

Display::Display(const std::string &title, int width, int height)
	: title(title), width(width), height(height) {
	window = SDL_CreateWindow(title.c_str(), width, height,
		SDL_WINDOW_OPENGL);

	if (window == nullptr) {
		error("Window creation failed.");
	}
	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED);

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
	if (gl_context != nullptr)
		SDL_GL_DestroyContext(gl_context);
	if (window != nullptr)
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

update_viewport:
	glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
	FrameBuffer::invalidate_bind_cache();
	FrameBuffer::resize_camera_sized_framebuffers(camera);
}

void Display::apply_screen_viewport() {
	glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
	FrameBuffer::invalidate_bind_cache();
}

void Display::set_fullscreen() {
	SDL_SetWindowFullscreen(window, true);
	int w = 0;
	int h = 0;
	SDL_GetWindowSize(window, &w, &h);
	update_viewport(w, h);
}

void Display::set_windowed(int width, int height) {
	SDL_SetWindowFullscreen(window, false);
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
