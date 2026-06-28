#pragma once

#include "engine/mathf.h"
#include <glm/glm.hpp>

class Engine;
class Texture;
class Shader;
class Mesh;

class GameObject {
      public:
        GameObject(Engine *engine, const Vector &position, float width, float height);
        GameObject(Engine *engine, float x, float y, float width, float height);

        Vector position;
        Vector anchor = Vector(0.5f, 0.5f);
        float width, height;
        float rotation = 0.0f;

        int slot;

        Texture *texture;
        Mesh *mesh = nullptr;
        Shader *shader;
        bool flip_x = false;
        bool flip_y = false;
        bool batchable = true;
        float render_offset_x = 0.0f;
        float render_offset_y = 0.0f;
        float render_padding_x = 0.0f;
        float render_padding_y = 0.0f;
        int blend_source;
        int blend_destination;

        virtual void init();
        virtual void update();
        void render();
        bool can_batch() const;
        void set_render_offset(float x, float y);
        void set_render_padding(float x, float y);
        void set_blend_mode(int source, int destination);
        int get_render_atlas_index() const;
        void set_render_atlas_index(int atlas_index);
        bool is_ui() const;
        void set_is_ui(bool is_ui);

        Engine *get_engine();

      private:
        Engine *engine;
        bool ui = false;
};
