#include "engine/engine.h"

#include "engine/camera.h"
#include "engine/display.h"
#include "engine/framebuffer.h"
#include "engine/input.h"
#include "engine/renderer.h"
#include "engine/scene.h"
#include "engine/shader.h"
#include "engine/shader_manager.h"

void Engine::init_engine(const std::string &title, int width, int height) {

        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
                printf("SDL_Init Error: %s\n", SDL_GetError());
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_CORE);

        this->width = width;
        this->height = height;

        Display *display = new Display(title, width, height);
        Camera *camera = new Camera((float)width, (float)height);
        Input *input = new Input();

        ShaderManager *shader_manager = new ShaderManager();

        Shader *default_unlit_shader =
            new Shader("shaders/default.frag", "shaders/default.vert");
        Shader *plain_shape_shader =
            new Shader("shaders/default.frag", "shaders/default.vert");

        shader_manager->fetch_shader("default_unlit", default_unlit_shader);
        shader_manager->fetch_shader("default_shape", plain_shape_shader);

        Renderer *renderer = new Renderer(
            this, camera, shader_manager->get_shader("default_unlit"));

        this->frame_buffer = nullptr;
        this->display = display;
        this->camera = camera;
        this->renderer = renderer;
        this->shader_manager = shader_manager;
        this->input = input;
}

float Engine::get_width() { return width; }

float Engine::get_height() { return height; }

Input *Engine::get_input() { return input; }

Scene *Engine::get_current_scene() { return current_scene; }

void Engine::set_scene(Scene *scene) {
        this->current_scene = scene;
        scene->init();
}

FrameBuffer *Engine::get_frame_buffer() { return frame_buffer; }

void Engine::set_frame_buffer(FrameBuffer *frame_buffer) {
        this->frame_buffer = frame_buffer;
}

void Engine::start() {
        bool running = true;
        Uint64 previous = SDL_GetPerformanceCounter();
        double lag = 0.0;

        const int TARGET_FPS = 60;
        const float TIME_STEP = 1.0f / TARGET_FPS;

        SDL_Event event;
        int frames = 0;
        int updates = 0;
        double fps_timer = 0.0;

        while (running) {
                input->begin_new_frame();

                while (SDL_PollEvent(&event)) {

                        input->process_event(event, this);

                        switch (event.type) {
                        case SDL_QUIT:
                                running = false;
                                break;

                        case SDL_WINDOWEVENT:
                                if (event.window.event ==
                                    SDL_WINDOWEVENT_SIZE_CHANGED) {
                                        int w = event.window.data1;
                                        int h = event.window.data2;
                                        this->display->update_viewport(w, h);
                                }
                        }
                }

                Uint64 current = SDL_GetPerformanceCounter();
                double elapsed = (double)(current - previous) /
                                 SDL_GetPerformanceFrequency();
                previous = current;
                lag += elapsed;
                fps_timer += elapsed;

                while (lag >= TIME_STEP) {
                        if (current_scene != nullptr) {
                                current_scene->update();
                        }
                        updates++;
                        lag -= TIME_STEP;
                }

                if (current_scene != nullptr) {
                        current_scene->render();
                        current_scene->flush_render_context();
                }

                frames++;
                SDL_GL_SwapWindow(display->get_window());

                if (fps_timer >= 1.0) {
                        printf("FPS: %d | UPS: %d\n", frames, updates);
                        frames = 0;
                        updates = 0;
                        fps_timer -= 1.0;
                }
        }
}

ShaderManager *Engine::get_shader_manager() { return this->shader_manager; }

Renderer *Engine::get_renderer() { return this->renderer; }

Camera *Engine::get_camera() { return this->camera; }

Display *Engine::get_display() {
        return this->display;
        ;
}