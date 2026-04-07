#include "engine/display.h"
#include "engine/error.h"

#include <thread>

Display::Display(const std::string &title, int width, int height)
    : title(title), width(width), height(height) {

        window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED, width, height,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

        if (this->window == NULL) {
                error("Window creation failed.");
        }

        this->gl_context = SDL_GL_CreateContext(window);
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
        SDL_DestroyWindow(this->window);
        SDL_Quit();
}

SDL_Window *Display::get_window() { return window; }

void Display::update_viewport(int win_width, int win_height) {
        float t_aspect = (float)width / height;
        float win_aspect = (float)win_width / win_height;

        if (win_aspect > t_aspect) {
                int new_width = (int)(win_height * t_aspect);
                int offset = (win_width - new_width) / 2;
                glViewport(offset, 0, new_width, new_width);
        } else {
                int new_height = (int)(win_width / t_aspect);
                int offset = (win_height - new_height) / 2;
                glViewport(0, offset, win_width, new_height);
        }
}

std::string Display::get_title() { return title; }

int Display::get_width() { return width; }

int Display::get_height() { return height; }