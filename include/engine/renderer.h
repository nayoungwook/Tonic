#pragma once

#include <glm/glm.hpp>

class Vector;
class Texture;
class Engine;
class FrameBuffer;
class Shader;
class Camera;

class Renderer {
      public:
        Camera *get_camera();

        Renderer(Engine *engine, Camera *camera, Shader *shader);

        void render_texture(FrameBuffer *frame_buffer, Texture *texture,
                            int slot, Shader *shader, Vector position,
                            float width, float height);
        void render_texture(FrameBuffer *frame_buffer, Texture *texture,
                            Shader *shader, Vector position, float width,
                            float height);

        Shader *get_shader();
        void set_shader(Shader *shader);

      private:
        Engine *engine;
        Shader *shader;
        Camera *camera;
};