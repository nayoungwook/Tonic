#pragma once

#include <GL/glew.h>
#include <SDL3/SDL.h>
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
        static void apply_current_screen_viewport();
        static Display *get_current();
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
        Camera *camera = nullptr;
};
