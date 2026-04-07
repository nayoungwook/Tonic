#pragma once

#include <iostream>

#include "engine/display.h"
#include "engine/renderer.h"
#include "engine/scene.h"
#include "engine/shader.h"
#include "engine/shader_manager.h"
#include "engine/texture.h"

class Display;
class Camera;
class Renderer;
class ShaderManager;
class Scene;
class Input;
class FrameBuffer;

class Engine {
      public:
        Display *get_display();
        Camera *get_camera();
        Renderer *get_renderer();
        ShaderManager *get_shader_manager();
        Scene *get_current_scene();
        Input *get_input();
        FrameBuffer *get_frame_buffer();

        float get_width(), get_height();

        void set_frame_buffer(FrameBuffer *frame_buffer);

        void set_scene(Scene *scene);
        void start();

        void init_engine(const std::string &title, int width, int height);

      private:
        float width, height;

        FrameBuffer *frame_buffer;
        Scene *current_scene;
        Display *display;
        Camera *camera;
        Renderer *renderer;
        ShaderManager *shader_manager;
        Input *input;
};