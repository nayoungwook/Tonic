#pragma once

#include <vector>

class FrameBuffer {
      public:
        enum class SizeMode {
                RENDER,
                CAMERA,
                FIXED
        };

        FrameBuffer();
        explicit FrameBuffer(bool pixel_perfect);
        explicit FrameBuffer(SizeMode size_mode);
        FrameBuffer(SizeMode size_mode, bool pixel_perfect);
        FrameBuffer(int width, int height);
        FrameBuffer(int width, int height, bool pixel_perfect);
        ~FrameBuffer();

        void bind();
        void unbind();
        void clear(float r = 0.0f, float g = 0.0f, float b = 0.0f,
                   float a = 1.0f);
        void resize(int width, int height);

        unsigned get_color_texture() const { return color_texture; }
        unsigned get_fbo() const { return fbo; }
        int get_width() const { return width; }
        int get_height() const { return height; }
        float get_resolution_x() const { return static_cast<float>(width); }
        float get_resolution_y() const { return static_cast<float>(height); }
        bool is_pixel_perfect() const { return pixel_perfect; }
        FrameBuffer *set_pixel_perfect(bool pixel_perfect);

        void create(int width, int height);
        void upload_texture(int tex);
        void bind_texture(int tex);
        void dispose();
        static void resize_camera_sized_framebuffers();

      private:
        void update_resolution_from_mode();
        void register_dynamic();
        void unregister_dynamic();

        unsigned fbo = 0;
        unsigned color_texture = 0;
        unsigned rbo = 0;

        int width = 0;
        int height = 0;
        bool pixel_perfect = false;
        bool disposed = false;
        SizeMode size_mode = SizeMode::FIXED;
        bool dynamic_registered = false;
};
