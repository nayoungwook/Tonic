#include "engine/framebuffer.h"

#include "engine/error.h"

#include <GL/glew.h>
#include <SDL/SDL.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>

namespace {
std::vector<FrameBuffer *> dynamic_framebuffers;

void current_drawable_size(int &width, int &height) {
        SDL_Window *window = SDL_GL_GetCurrentWindow();
        if (window != nullptr) {
                SDL_GL_GetDrawableSize(window, &width, &height);
        }
        width = std::max(1, width);
        height = std::max(1, height);
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
}

void FrameBuffer::bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, width, height);
}

void FrameBuffer::unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void FrameBuffer::clear(float r, float g, float b, float a) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
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

void FrameBuffer::update_resolution_from_mode() {
        if (size_mode == SizeMode::FIXED)
                return;

        int drawable_width = width;
        int drawable_height = height;
        current_drawable_size(drawable_width, drawable_height);
        width = drawable_width;
        height = drawable_height;
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
                int new_width = framebuffer->width;
                int new_height = framebuffer->height;
                current_drawable_size(new_width, new_height);
                framebuffer->resize(new_width, new_height);
        }
}
