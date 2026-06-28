#include "engine/framebuffer.h"

#include "engine/camera.h"
#include "engine/error.h"

#include <GL/glew.h>
#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>

namespace {
	std::vector<FrameBuffer *> dynamic_framebuffers;
	unsigned cached_bound_fbo = static_cast<unsigned>(-1);
	int cached_viewport_width = -1;
	int cached_viewport_height = -1;
	constexpr int ENGINE_MAX_FRAMEBUFFER_SIZE = 8192;

	void current_drawable_size(int &width, int &height) {
		SDL_Window *window = SDL_GL_GetCurrentWindow();
		if (window != nullptr) {
			SDL_GetWindowSizeInPixels(window, &width, &height);
		}
		width = std::max(1, width);
		height = std::max(1, height);
	}

	void bind_fbo(unsigned fbo) {
		if (cached_bound_fbo == fbo)
			return;
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		cached_bound_fbo = fbo;
	}

	void set_viewport(int width, int height) {
		if (cached_viewport_width == width &&
			cached_viewport_height == height)
			return;
		glViewport(0, 0, width, height);
		cached_viewport_width = width;
		cached_viewport_height = height;
	}

	int get_max_framebuffer_size() {
		int max_texture_size = ENGINE_MAX_FRAMEBUFFER_SIZE;
		int max_renderbuffer_size = ENGINE_MAX_FRAMEBUFFER_SIZE;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
		glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &max_renderbuffer_size);
		max_texture_size = std::max(1, max_texture_size);
		max_renderbuffer_size = std::max(1, max_renderbuffer_size);
		return std::max(1, std::min({ ENGINE_MAX_FRAMEBUFFER_SIZE,
			max_texture_size, max_renderbuffer_size }));
	}

	void clamp_framebuffer_size(int &width, int &height) {
		int max_size = get_max_framebuffer_size();
		width = std::clamp(width, 1, max_size);
		height = std::clamp(height, 1, max_size);
	}

	int ceil_to_clamped_size(double value, int max_size) {
		if (!std::isfinite(value))
			return max_size;
		if (value <= 1.0)
			return 1;
		if (value >= static_cast<double>(max_size))
			return max_size;
		return std::max(1, static_cast<int>(std::ceil(value)));
	}
}

FrameBuffer::FrameBuffer() : FrameBuffer(SizeMode::RENDER) {
	this->pixel_perfect = false;
}

FrameBuffer::FrameBuffer(bool pixel_perfect)
	: FrameBuffer(SizeMode::RENDER, pixel_perfect) {}

FrameBuffer::FrameBuffer(SizeMode size_mode)
	: FrameBuffer(size_mode, false) {}

FrameBuffer::FrameBuffer(SizeMode size_mode, bool pixel_perfect)
	: pixel_perfect(pixel_perfect), size_mode(size_mode) {
	update_resolution_from_mode();
	create(width, height);
	register_dynamic();
}

FrameBuffer::FrameBuffer(int width, int height)
	: FrameBuffer(width, height, false) {}

FrameBuffer::FrameBuffer(int width, int height, bool pixel_perfect)
	: pixel_perfect(pixel_perfect), size_mode(SizeMode::FIXED) {
	this->pixel_view_width = std::max(1, width);
	this->pixel_view_height = std::max(1, height);
	create(std::max(1, width), std::max(1, height));
}

void FrameBuffer::set_pixel_per_unit(float pixel, float world_size) {
	if (pixel <= 0.0f || world_size <= 0.0f)
		return;
	this->pixel_per_unit = world_size / pixel;
	int old_width = width;
	int old_height = height;
	update_resolution_from_mode();
	if (old_width != width || old_height != height)
		create(width, height);
}

float FrameBuffer::get_pixel_per_unit() {
	return this->pixel_per_unit;
}

bool FrameBuffer::is_pixel_perfect() const { return pixel_perfect; }

FrameBuffer::~FrameBuffer() {
	unregister_dynamic();
	dispose();
}

void FrameBuffer::dispose() {
	if (disposed)
		return;
	if (rbo != 0)
		glDeleteRenderbuffers(1, &rbo);
	if (color_texture != 0)
		glDeleteTextures(1, &color_texture);
	if (fbo != 0)
		glDeleteFramebuffers(1, &fbo);
	rbo = 0;
	color_texture = 0;
	fbo = 0;
	disposed = true;
}

void FrameBuffer::create(int width, int height) {
	clamp_framebuffer_size(width, height);

	if (rbo != 0) {
		glDeleteRenderbuffers(1, &rbo);
		rbo = 0;
	}
	if (color_texture != 0) {
		glDeleteTextures(1, &color_texture);
		color_texture = 0;
	}
	if (fbo != 0) {
		glDeleteFramebuffers(1, &fbo);
		fbo = 0;
	}

	this->width = width;
	this->height = height;
	this->disposed = false;

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	cached_bound_fbo = fbo;

	glGenTextures(1, &color_texture);
	glBindTexture(GL_TEXTURE_2D, color_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, nullptr);

	GLenum filter = pixel_perfect ? GL_NEAREST : GL_LINEAR;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, color_texture, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width,
		height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
		GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
		GL_FRAMEBUFFER_COMPLETE) {
		error("Framebuffer is not complete!");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	cached_bound_fbo = 0;
	cached_viewport_width = -1;
	cached_viewport_height = -1;
}

void FrameBuffer::bind() {
	bind_fbo(fbo);
	set_viewport(width, height);
}

void FrameBuffer::unbind() { bind_screen_framebuffer(); }

void FrameBuffer::bind_screen_framebuffer() { bind_fbo(0); }

void FrameBuffer::invalidate_bind_cache() {
	cached_bound_fbo = static_cast<unsigned>(-1);
	cached_viewport_width = -1;
	cached_viewport_height = -1;
}

void FrameBuffer::clear(float r, float g, float b, float a) {
	bind();
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void FrameBuffer::resize(int width, int height) {
	width = std::max(1, width);
	height = std::max(1, height);
	if (width == this->width && height == this->height)
		return;
	if (size_mode == SizeMode::FIXED || !pixel_perfect) {
		pixel_view_width = width;
		pixel_view_height = height;
	}
	create(width, height);
}

void FrameBuffer::upload_texture(int tex) {
	glActiveTexture(GL_TEXTURE0 + tex);
	glBindTexture(GL_TEXTURE_2D, color_texture);
}

void FrameBuffer::bind_texture(int tex) { upload_texture(tex); }

void FrameBuffer::update_resolution_from_mode(Camera *camera) {
	if (size_mode == SizeMode::FIXED)
		return;

	int target_width = width;
	int target_height = height;

	if (camera != nullptr) {
		target_width = camera->get_width();
		target_height = camera->get_height();
	}
	else {
		current_drawable_size(target_width, target_height);
	}

	reference_width = std::max(1, target_width);
	reference_height = std::max(1, target_height);

	if (!pixel_perfect) {
		width = reference_width;
		height = reference_height;
		pixel_view_width = width;
		pixel_view_height = height;
		return;
	}

	float units_per_pixel = std::max(0.0001f, pixel_per_unit);
	int max_size = get_max_framebuffer_size();
	int max_pixel_view_size = std::max(1, max_size - 2);
	pixel_view_width = ceil_to_clamped_size(
		static_cast<double>(reference_width) / units_per_pixel,
		max_pixel_view_size);
	pixel_view_height = ceil_to_clamped_size(
		static_cast<double>(reference_height) / units_per_pixel,
		max_pixel_view_size);

	float zoom = camera == nullptr ? 1.0f : std::max(0.01f,
		std::abs(camera->zoom));
	double visible_diameter =
		std::hypot(static_cast<double>(pixel_view_width),
			static_cast<double>(pixel_view_height)) / zoom;

	width = std::min(max_size, std::max(pixel_view_width + 2,
		ceil_to_clamped_size(visible_diameter + 2.0, max_size)));
	height = std::min(max_size, std::max(pixel_view_height + 2,
		ceil_to_clamped_size(visible_diameter + 2.0, max_size)));
}

void FrameBuffer::register_dynamic() {
	if (size_mode == SizeMode::FIXED || dynamic_registered)
		return;
	dynamic_framebuffers.push_back(this);
	dynamic_registered = true;
}

void FrameBuffer::unregister_dynamic() {
	if (!dynamic_registered)
		return;
	auto it = std::find(dynamic_framebuffers.begin(),
		dynamic_framebuffers.end(), this);
	if (it != dynamic_framebuffers.end())
		dynamic_framebuffers.erase(it);
	dynamic_registered = false;
}

void FrameBuffer::resize_camera_sized_framebuffers() {
	resize_camera_sized_framebuffers(nullptr);
}

void FrameBuffer::resize_camera_sized_framebuffers(Camera *camera) {
	if (camera != nullptr)
		camera->configure_pixel_perfect_transform(false);

	for (FrameBuffer *framebuffer : dynamic_framebuffers) {
		if (framebuffer == nullptr)
			continue;
		int old_width = framebuffer->width;
		int old_height = framebuffer->height;
		framebuffer->update_resolution_from_mode(camera);
		if (camera != nullptr && framebuffer->is_pixel_perfect()) {
			camera->configure_pixel_perfect_transform(true,
				framebuffer->get_pixel_per_unit(),
				static_cast<float>(framebuffer->get_pixel_view_width()),
				static_cast<float>(framebuffer->get_pixel_view_height()));
		}
		if (old_width != framebuffer->width ||
			old_height != framebuffer->height) {
			framebuffer->create(framebuffer->width,
				framebuffer->height);
		}
	}
}
