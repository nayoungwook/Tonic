#pragma once

#include <vector>

#include "engine/render_context.h"
#include "engine/util.h"

class Engine;

class Scene {
      public:
        Scene(Engine *engine);

        virtual void init() = 0;
        virtual void update() = 0;
        virtual void render() = 0;

        void flush_render_context();
        void add_render_context(const RenderContext &render_context);

        void start_frame();
        Engine *get_engine();

      protected:
        Engine *engine;

      private:
        Rect get_camera_view(float cam_x, float cam_y, float screen_w,
                             float screen_h);

        Rect compute_object_aabb_fast(const RenderContext &rc);

        bool intersects(const Rect &a, const Rect &b);

        std::vector<RenderContext> render_context_queue;

        Rect cached_camera_view;
        bool camera_dirty = true;
};

void set_scene(Scene *scene);