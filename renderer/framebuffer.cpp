#include "engine/framebuffer.h"

#include "engine/error.h"

#include <GL/glew.h>
#include <SDL/SDL.h>

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

void current_drawable_size(int &width, int &height) {
        SDL_Window *window = SDL_GL_GetCurrentWindow();
        if (window != nullptr) {
                SDL_GL_GetDrawableSize(window, &width, &height);
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
}

FrameBuffer::FrameBuffer() : FrameBuffer(SizeMode::RENDER, false) {}

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
        create(std::max(1, width), std::max(1, height));
}

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
        if (width == this->width && height == this->height)
                return;
        create(width, height);
}

void FrameBuffer::upload_texture(int tex) {
        glActiveTexture(GL_TEXTURE0 + tex);
        glBindTexture(GL_TEXTURE_2D, color_texture);
}

void FrameBuffer::bind_texture(int tex) { upload_texture(tex); }

FrameBuffer *FrameBuffer::set_pixel_perfect(bool pixel_perfect) {
        this->pixel_perfect = pixel_perfect;
        if (color_texture != 0) {
                glBindTexture(GL_TEXTURE_2D, color_texture);
                GLenum filter = pixel_perfect ? GL_NEAREST : GL_LINEAR;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
        }
        return this;
}

FrameBuffer *FrameBuffer::configure_pixel_perfect(float reference_width,
        float reference_height, float assets_pixels_per_unit) {
        if (reference_width <= 0.0f || reference_height <= 0.0f ||
            assets_pixels_per_unit <= 0.0f)
                return this;

        this->reference_width = std::max(1,
                static_cast<int>(std::round(reference_width)));
        this->reference_height = std::max(1,
                static_cast<int>(std::round(reference_height)));
        this->assets_pixels_per_unit =
                std::max(0.0001f, assets_pixels_per_unit);
        set_pixel_perfect(true);
        int old_width = width;
        int old_height = height;
        update_resolution_from_mode();
        if (old_width != width || old_height != height)
                create(width, height);
        return this;
}

FrameBuffer *FrameBuffer::configure_pixel_perfect_for_sprite(
        int sprite_pixel_size, float world_size) {
        if (sprite_pixel_size <= 0 || world_size <= 0.0f)
                return this;

        int drawable_width = width;
        int drawable_height = height;
        current_drawable_size(drawable_width, drawable_height);
        return configure_pixel_perfect(static_cast<float>(drawable_width),
                static_cast<float>(drawable_height),
                static_cast<float>(sprite_pixel_size) / world_size);
}

float FrameBuffer::get_world_units_per_pixel() const {
        return 1.0f / std::max(0.0001f, assets_pixels_per_unit);
}

int FrameBuffer::get_pixel_view_width() const {
        int source_width = reference_width > 0 ? reference_width : width;
        return std::max(1, static_cast<int>(std::ceil(
                static_cast<float>(source_width) * assets_pixels_per_unit)));
}

int FrameBuffer::get_pixel_view_height() const {
        int source_height = reference_height > 0 ? reference_height : height;
        return std::max(1, static_cast<int>(std::ceil(
                static_cast<float>(source_height) * assets_pixels_per_unit)));
}

int FrameBuffer::get_pixel_buffer_width() const {
        return get_pixel_view_width() + 1;
}

int FrameBuffer::get_pixel_buffer_height() const {
        return get_pixel_view_height() + 1;
}

glm::vec4 FrameBuffer::get_pixel_source_uv(float camera_x,
        float camera_y) const {
        float units_per_pixel = get_world_units_per_pixel();
        int source_width = reference_width > 0 ? reference_width : width;
        int source_height = reference_height > 0 ? reference_height : height;

        float left = camera_x - static_cast<float>(source_width) * 0.5f;
        float bottom = camera_y - static_cast<float>(source_height) * 0.5f;
        float snapped_left = std::floor(left / units_per_pixel) *
                units_per_pixel;
        float snapped_bottom = std::floor(bottom / units_per_pixel) *
                units_per_pixel;
        float frac_x = (left - snapped_left) / units_per_pixel;
        float frac_y = (bottom - snapped_bottom) / units_per_pixel;

        float buffer_width = static_cast<float>(get_pixel_buffer_width());
        float buffer_height = static_cast<float>(get_pixel_buffer_height());
        float view_width = static_cast<float>(get_pixel_view_width());
        float view_height = static_cast<float>(get_pixel_view_height());

        return glm::vec4(frac_x / buffer_width, frac_y / buffer_height,
                (frac_x + view_width) / buffer_width,
                (frac_y + view_height) / buffer_height);
}

void FrameBuffer::update_resolution_from_mode() {
        if (size_mode == SizeMode::FIXED)
                return;

        int drawable_width = width;
        int drawable_height = height;
        current_drawable_size(drawable_width, drawable_height);
        if (reference_width <= 0)
                reference_width = drawable_width;
        if (reference_height <= 0)
                reference_height = drawable_height;

        if (pixel_perfect) {
                width = get_pixel_buffer_width();
                height = get_pixel_buffer_height();
        } else {
                width = drawable_width;
                height = drawable_height;
        }
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
        for (FrameBuffer *framebuffer : dynamic_framebuffers) {
                if (framebuffer == nullptr)
                        continue;
                int old_width = framebuffer->width;
                int old_height = framebuffer->height;
                framebuffer->update_resolution_from_mode();
                if (old_width != framebuffer->width ||
                    old_height != framebuffer->height) {
                        framebuffer->create(framebuffer->width,
                                framebuffer->height);
                }
        }
}
