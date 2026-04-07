#pragma once

#include <GL/glew.h>
#include <SDL/SDL.h>
#include <iostream>

class Display {
      public:
        Display(const std::string &title, int width, int height);
        ~Display();
        std::string get_title();
        int get_width();
        int get_height();

        void update_viewport(int win_width, int win_height);
        void start();

        SDL_Window *get_window();

      private:
        SDL_GLContext gl_context;
        SDL_Window *window = nullptr;
        std::string title;
        int width, height;
};