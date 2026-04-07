#include "engine/game_object.h"
#include "engine/engine.h"
#include "engine/error.h"
#include "engine/render_context.h"
#include "engine/renderer.h"
#include "engine/scene.h"

GameObject::GameObject(Engine *engine, Vector &position, float width,
                       float height)
    : engine(engine), position(position), width(width), height(height) {
        this->init();

        this->z_order = 1;
        this->slot = 0;
}

void GameObject::render() {
        Scene *current_scene = this->engine->get_current_scene();
        if (current_scene == nullptr) {
                error("current scene is null");
        }

        Shader *shader = this->engine->get_renderer()->get_shader();
        RenderContext rc = gen_render_context(
            this->engine->get_frame_buffer(), this->texture, shader,
            this->position, this->slot, this->rotation, this->width,
            this->height, this->z_order);
        current_scene->add_render_context(rc);
}

void GameObject::init() {}
void GameObject::update() {}