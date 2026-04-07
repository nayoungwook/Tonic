#include "engine/renderer.h"
#include "engine/camera.h"
#include "engine/engine.h"
#include "engine/render_context.h"
#include "engine/scene.h"
#include "engine/mathf.h"

Renderer::Renderer(Engine *engine, Camera *camera, Shader *shader)
    : engine(engine), camera(camera), shader(shader) {}

void Renderer::set_shader(Shader *shader) { this->shader = shader; }

void Renderer::render_texture(FrameBuffer *frame_buffer, Texture *texture,
                              int slot, Shader *shader, Vector position,
                              float width, float height) {
        RenderContext rc = gen_render_context(
            frame_buffer, texture, shader, position, slot, 0, width, height, 1);
        engine->get_current_scene()->add_render_context(rc);
}

void Renderer::render_texture(FrameBuffer *frame_buffer, Texture *texture,
                              Shader *shader, Vector position, float width,
                              float height) {
        RenderContext rc = gen_render_context(frame_buffer, texture, shader,
                                              position, 0, 0, width, height, 1);
        engine->get_current_scene()->add_render_context(rc);
}

Camera *Renderer::get_camera() { return camera; }

Shader *Renderer::get_shader() { return shader; }