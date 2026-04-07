#include <iostream>

#include "engine/engine.h"

class Game : public Scene {
      private:
        Texture *texture = nullptr;
        Shader *default_unlit = nullptr;
        Renderer *renderer = nullptr;

      public:
        Game(Engine *engine) : Scene(engine) {}

        void init() {
                texture = new Texture("test/res/aru.png");
                ShaderManager *shader_manager =
                    this->engine->get_shader_manager();
                default_unlit = shader_manager->get_shader("default_unlit");
                renderer = this->engine->get_renderer();
        }

        void update() {}

        void render() {
                renderer->render_texture(this->engine->get_frame_buffer(),
                                         texture, default_unlit, Vector(0, 0),
                                         500, 500);
        }
};

int main(int argc, char *argv[]) {
        int width = 1280, height = 720;

        Engine *engine = new Engine();
        engine->init_engine("title", width, height);

        engine->set_scene(new Game(engine));

        engine->start();

        return 0;
}