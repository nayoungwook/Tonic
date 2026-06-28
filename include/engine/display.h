#pragma once

#include <GL/glew.h>
#include <SDL/SDL.h>
#include <iostream>

class Camera;

class Display {
      public:
        Display(const std::string &title, int width, int height);
        ~Display();
        std::string get_title();
        int get_width();
        int get_height();

        void update_viewport(int win_width, int win_height);
        void apply_screen_viewport();
        void set_pixel_perfect_screen(bool enabled);
        bool is_pixel_perfect_screen() const;
        void configure_pixel_perfect(int reference_width,
                int reference_height, int assets_pixels_per_unit);
        void configure_pixel_perfect_for_sprite(int sprite_pixel_size,
                float world_size);
        void set_pixel_snapping(bool enabled);
        void set_fullscreen();
        void set_windowed(int width, int height);
        void start();

        SDL_Window *get_window();
        void set_camera(Camera *camera);
        int get_viewport_x() const;
        int get_viewport_y() const;
        int get_viewport_width() const;
        int get_viewport_height() const;

      private:
        SDL_GLContext gl_context;
        SDL_Window *window = nullptr;
        std::string title;
        int width, height;
        int viewport_x = 0;
        int viewport_y = 0;
        int viewport_width = 0;
        int viewport_height = 0;
        bool pixel_perfect_screen = false;
        Camera *camera = nullptr;
};
