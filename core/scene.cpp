#include "engine/scene.h"
#include "engine/framebuffer.h"
#include "engine/render_context.h"
#include "engine/texture.h"

#include "engine/camera.h"
#include "engine/engine.h"
#include "engine/renderer.h"
#include "engine/shader.h"

#include <algorithm>
#include <cfloat>
#include <cmath>

#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>

Scene::Scene(Engine *engine) : engine(engine) {}

Engine *Scene::get_engine() { return engine; }

Rect Scene::get_camera_view(float cam_x, float cam_y, float screen_w,
                            float screen_h) {
        Rect view;
        view.left = cam_x - screen_w * 0.5f;
        view.right = cam_x + screen_w * 0.5f;
        view.bottom = cam_y - screen_h * 0.5f;
        view.top = cam_y + screen_h * 0.5f;
        return view;
}

Rect Scene::compute_object_aabb_fast(const RenderContext &rc) {
        float hw = rc.width * 0.5f;
        float hh = rc.height * 0.5f;

        Rect r;
        r.left = rc.position.x - hw;
        r.right = rc.position.x + hw;
        r.bottom = rc.position.y - hh;
        r.top = rc.position.y + hh;

        return r;
}

inline bool Scene::intersects(const Rect &a, const Rect &b) {
        return !(a.right < b.left || a.left > b.right || a.top < b.bottom ||
                 a.bottom > b.top);
}

void Scene::flush_render_context() {
        if (render_context_queue.empty())
                return;

        std::sort(render_context_queue.begin(), render_context_queue.end(),
                  [](const RenderContext &a, const RenderContext &b) {
                          return a.z_order < b.z_order;
                  });

        Camera *camera = engine->get_camera();
        Renderer *renderer = engine->get_renderer();

        camera->calculate_matrix();

        Shader *shader_cache = nullptr;

        for (const RenderContext &rc : render_context_queue) {

                Shader *shader = rc.shader;
                FrameBuffer *frame_buffer = rc.frame_buffer;

                if (frame_buffer == nullptr) {
                        glBindFramebuffer(GL_FRAMEBUFFER, 0);
                }

                if (frame_buffer != nullptr) {
                        frame_buffer->bind();
                }

                glm::mat4 model(1.0f);
                model = glm::translate(
                    model, glm::vec3(rc.position.x, rc.position.y, 0));
                model = glm::rotate(model, rc.rotation, glm::vec3(0, 0, 1));
                model = glm::scale(model, glm::vec3(rc.width, rc.height, 1.0f));

                if (shader_cache != shader) {
                        glUniform1i(shader->get_uniform_location("uTexture"),
                                    0);
                        glUniformMatrix4fv(
                            shader->get_uniform_location("uViewProjection"), 1,
                            GL_FALSE,
                            glm::value_ptr(camera->get_view_projection()));
                        renderer->set_shader(shader);
                        shader_cache = shader;
                }
                rc.texture->set_atlas(rc.slot);

                switch (rc.render_type) {
                case RT_TEXTURE:
                        rc.texture->render(renderer, model);
                        break;
                }
        }

        render_context_queue.clear();
}

void Scene::add_render_context(const RenderContext &render_context) {
        static Rect cached_camera_view;

        Camera *camera = engine->get_camera();
        bool camera_dirty = camera->is_dirty();

        if (camera_dirty) {
                cached_camera_view =
                    get_camera_view(camera->position.x, camera->position.y,
                                    camera->get_width(), camera->get_height());
        }

        Rect object_aabb = compute_object_aabb_fast(render_context);

        if (!intersects(cached_camera_view, object_aabb)) {
                return;
        }

        render_context_queue.push_back(render_context);
}

void Scene::start_frame() { camera_dirty = true; }
