#pragma once

#include <glm/glm.hpp>

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
        static void bind_screen_framebuffer();
        static void invalidate_bind_cache();
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
        FrameBuffer *configure_pixel_perfect(float reference_width,
                float reference_height, float assets_pixels_per_unit);
        FrameBuffer *configure_pixel_perfect_for_sprite(int sprite_pixel_size,
                float world_size);
        float get_assets_pixels_per_unit() const { return assets_pixels_per_unit; }
        float get_world_units_per_pixel() const;
        int get_pixel_view_width() const;
        int get_pixel_view_height() const;
        int get_pixel_buffer_width() const;
        int get_pixel_buffer_height() const;
        int get_reference_width() const { return reference_width > 0 ? reference_width : width; }
        int get_reference_height() const { return reference_height > 0 ? reference_height : height; }
        glm::vec4 get_pixel_source_uv(float camera_x, float camera_y) const;

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
        int reference_width = 0;
        int reference_height = 0;
        float assets_pixels_per_unit = 1.0f;
        bool pixel_perfect = false;
        bool disposed = false;
        SizeMode size_mode = SizeMode::FIXED;
        bool dynamic_registered = false;
};
