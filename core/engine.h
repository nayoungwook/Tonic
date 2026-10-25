#pragma once

#include <iostream>
#include <functional>

#include "camera.h"
#include "display.h"
#include "renderer.h"
#include "scene.h"
#include "shader.h"
#include "shader_manager.h"
#include "texture.h"
#include "input.h"
#include "sound.h"
#include "mesh.h"
#include "ttfont.h"
#include "screen_quad.h"

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
        FrameBuffer *get_framebuffer();

        float get_width(), get_height();

        void set_framebuffer(FrameBuffer *framebuffer);
        void set_automatic_presentation(bool enabled);
        bool is_automatic_presentation() const;

        void set_scene(Scene *scene);
		void set_after_scene_update(std::function<void(Engine *)> callback);
        void start();

        void init_engine(const std::string &title, int width, int height);

        int get_frames();
        int get_ticks();

      private:
        float width, height;

        int cached_frames, cached_ticks;

        FrameBuffer *current_framebuffer = nullptr;
        Scene *current_scene = nullptr;
		std::function<void(Engine *)> after_scene_update;
        Display *display = nullptr;
        Camera *camera = nullptr;
        Renderer *renderer = nullptr;
        ShaderManager *shader_manager = nullptr;
        Input *input = nullptr;
        bool automatic_presentation = true;
};
