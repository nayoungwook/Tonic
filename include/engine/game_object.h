#pragma once

#include "engine/mathf.h"
#include <glm/glm.hpp>

class Engine;
class Texture;
class Shader;

class GameObject {
      public:
        GameObject(Engine *engine, Vector &position, float width, float height);

        Vector position;
        float width, height;
        float rotation;
        float z_order;

        int slot;

        Texture *texture;
        Shader *shader;

        virtual void init();
        virtual void update();
        void render();

        Engine *get_engine();

      private:
        Engine *engine;
};