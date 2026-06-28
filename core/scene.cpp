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
#include <functional>

#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace {
	bool render_context_less(const RenderContext &a, const RenderContext &b) {
		if (a.position.z != b.position.z)
			return a.position.z < b.position.z;
		if (a.render_type != b.render_type)
			return a.render_type < b.render_type;
		if (a.frame_buffer != b.frame_buffer)
			return std::less<FrameBuffer *>()(a.frame_buffer, b.frame_buffer);
		if (a.is_ui != b.is_ui)
			return a.is_ui < b.is_ui;
		if (a.shader != b.shader)
			return std::less<Shader *>()(a.shader, b.shader);
		if (a.texture != b.texture)
			return std::less<Texture *>()(a.texture, b.texture);
		return a.slot < b.slot;
	}

	void sort_render_context_segments(std::vector<RenderContext> &queue) {
		auto segment_begin = queue.begin();

		for (auto it = queue.begin(); it != queue.end(); ++it) {
			if (it->render_type != RT_CLEAR)
				continue;

			if (segment_begin != it) {
				std::stable_sort(segment_begin, it, render_context_less);
			}
			segment_begin = it + 1;
		}

		if (segment_begin != queue.end()) {
			std::stable_sort(segment_begin, queue.end(),
				render_context_less);
		}
	}
}

Scene::Scene(Engine *engine) : engine(engine) {
	render_context_queue.reserve(65536);
	instance_batch.reserve(65536);
	render_context_queue_needs_sort = true;
}

Engine *Scene::get_engine() { return engine; }
Rect Scene::get_camera_view(float cam_x,
	float cam_y,
	float zoom,
	float screen_w,
	float screen_h)
{
	const float half_w = screen_w / zoom;
	const float half_h = screen_h / zoom;

	Rect view;
	view.left = cam_x - half_w;
	view.right = cam_x + half_w;
	view.bottom = cam_y - half_h;
	view.top = cam_y + half_h;

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

void Scene::flush_batch(Renderer *renderer,
	std::vector<TextureInstance> &instance_batch,
	Texture *batch_texture, int &batch_slot) {
	if (instance_batch.empty())
		return;

	if (batch_slot >= 0)
		batch_texture->set_atlas(batch_slot);
	batch_texture->render_instanced(
		renderer, instance_batch.data(),
		static_cast<int>(instance_batch.size()));
	instance_batch.clear();
}

inline void Scene::bind_frame_buffer(FrameBuffer *frame_buffer) {
	if (frame_buffer != nullptr) {
		frame_buffer->bind();
		frame_buffer_cache = frame_buffer;
		screen_frame_buffer_bound = false;
	}
	else if (!screen_frame_buffer_bound) {
		FrameBuffer::bind_screen_framebuffer();
		frame_buffer_cache = nullptr;
		screen_frame_buffer_bound = true;
	}
}

inline void Scene::rt_clear(Renderer *renderer) {
	flush_batch(renderer, instance_batch, batch_texture,
		batch_slot);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

inline void Scene::rt_shape(const RenderContext &rc, const glm::mat4 &view_projection, Renderer *renderer, FrameBuffer *frame_buffer) {
	flush_batch(renderer, instance_batch, batch_texture,
		batch_slot);
	bind_frame_buffer(frame_buffer);
	glBlendFunc(rc.blend_source, rc.blend_destination);
	renderer->draw_shape(rc, view_projection);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

inline void Scene::rt_frame_buffer(const RenderContext &rc, const glm::mat4 &view_projection, Renderer *renderer, FrameBuffer *frame_buffer) {
	flush_batch(renderer, instance_batch, batch_texture,
		batch_slot);
	bind_frame_buffer(frame_buffer);
	glBlendFunc(rc.blend_source, rc.blend_destination);
	renderer->draw_raw_texture(rc.raw_texture, rc, view_projection);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

inline void Scene::rt_mesh(const RenderContext &rc, const glm::mat4 &view_projection, Renderer *renderer, FrameBuffer *frame_buffer) {
	flush_batch(renderer, instance_batch, batch_texture,
		batch_slot);
	bind_frame_buffer(frame_buffer);
	glBlendFunc(rc.blend_source, rc.blend_destination);
	renderer->draw_mesh(rc, view_projection);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Scene::flush_render_context() {
	if (render_context_queue.empty())
		return;

	// Render Context batch sort order
	// z_order , render_type (RT_Image, RT_Mesh...), is_ui, frame_buffer, shader, texture, slot
	if (render_context_queue_needs_sort) {
		sort_render_context_segments(render_context_queue);
	}

	Camera *camera = engine->get_camera();
	Renderer *renderer = engine->get_renderer();

	// cache for shaders and framebuffer
	shader_cache = nullptr;
	shader_cache_is_ui = false;
	shader_cache_frame_buffer = nullptr;
	frame_buffer_cache = nullptr;

	texture_uniform_location = -1;
	screen_frame_buffer_bound = false;
	view_projection_uniform_location = -1;

	// batch data
	batch_frame_buffer = nullptr;
	batch_texture = nullptr;
	batch_shader = nullptr;
	batch_slot = 0;
	batch_is_ui = false;
	batch_blend_source = GL_SRC_ALPHA;
	batch_blend_destination = GL_ONE_MINUS_SRC_ALPHA;

	camera->calculate_matrix();

	bool pixel_perfect_cache = false;
	bool has_view_projection_cache = false;
	FrameBuffer *view_projection_cache_frame_buffer = nullptr;
	bool view_projection_cache_is_ui = false;
	glm::mat4 cached_view_projection(1.0f);

	for (const RenderContext &rc : render_context_queue) {

		Shader *shader = rc.shader;
		FrameBuffer *frame_buffer = rc.frame_buffer;

		bool pixel_perfect = (frame_buffer != nullptr && frame_buffer->is_pixel_perfect());

		if (!has_view_projection_cache ||
			view_projection_cache_frame_buffer != frame_buffer ||
			pixel_perfect != pixel_perfect_cache ||
			view_projection_cache_is_ui != rc.is_ui) {

			// update camera view_projection
			if (rc.is_ui) {
				cached_view_projection = camera->get_screen_projection();
			}
			else {
				cached_view_projection = camera->get_view_projection();
			}

			if (pixel_perfect) {
				cached_view_projection =
					camera->get_pixel_perfect_view_projection(
						frame_buffer->get_pixel_per_unit(),
						static_cast<float>(frame_buffer->get_width()),
						static_cast<float>(frame_buffer->get_height()));
			}

			has_view_projection_cache = true;
			view_projection_cache_frame_buffer = frame_buffer;
			view_projection_cache_is_ui = rc.is_ui;
			pixel_perfect_cache = pixel_perfect;
		}

		if (rc.render_type == RT_CLEAR) {
			this->rt_clear(renderer);
			continue;
		}

		if (rc.render_type == RT_SHAPE) {
			this->rt_shape(rc, cached_view_projection, renderer,
				frame_buffer);
			continue;
		}

		if (rc.render_type == RT_FRAMEBUFFER) {
			this->rt_frame_buffer(rc, cached_view_projection, renderer,
				frame_buffer);
			continue;
		}

		if (rc.render_type == RT_MESH) {
			this->rt_mesh(rc, cached_view_projection, renderer,
				frame_buffer);
			continue;
		}

		// change frame buffer
		if (frame_buffer != nullptr &&
			frame_buffer_cache != frame_buffer) {
			flush_batch(renderer, instance_batch, batch_texture,
				batch_slot);
			bind_frame_buffer(frame_buffer);
			frame_buffer_cache = frame_buffer;
			screen_frame_buffer_bound = false;
		}

		// update shader
		if (shader_cache != shader || shader_cache_is_ui != rc.is_ui ||
			shader_cache_frame_buffer != frame_buffer) {
			flush_batch(renderer, instance_batch, batch_texture,
				batch_slot);
			shader->bind();
			texture_uniform_location =
				shader->get_uniform_location("uTexture");
			view_projection_uniform_location =
				shader->get_uniform_location("uViewProjection");

			if (texture_uniform_location != -1) {
				glUniform1i(texture_uniform_location, 0);
			}

			if (view_projection_uniform_location != -1) {
				glUniformMatrix4fv(
					view_projection_uniform_location, 1,
					GL_FALSE,
					glm::value_ptr(cached_view_projection));
			}

			renderer->set_shader(shader);
			shader_cache = shader;
			shader_cache_is_ui = rc.is_ui;
			shader_cache_frame_buffer = frame_buffer;
		}

		if (rc.render_type == RT_TEXTURE) {
			bool is_batch_changed =
				batch_texture != rc.texture ||
				batch_shader != shader ||
				batch_frame_buffer != frame_buffer ||
				batch_slot != rc.slot ||
				batch_is_ui != rc.is_ui ||
				batch_blend_source != rc.blend_source ||
				batch_blend_destination != rc.blend_destination;

			if (is_batch_changed) {
				flush_batch(renderer, instance_batch,
					batch_texture, batch_slot);

				// cache batch data.
				batch_texture = rc.texture;
				batch_shader = shader;
				batch_frame_buffer = frame_buffer;
				batch_slot = rc.slot;
				batch_is_ui = rc.is_ui;
				batch_blend_source = rc.blend_source;
				batch_blend_destination = rc.blend_destination;
				glBlendFunc(batch_blend_source,
					batch_blend_destination);
			}

			instance_batch.push_back({ {rc.position.x, rc.position.y,
									   rc.width, rc.height},
									  rc.color,
									  rc.texture->get_atlas_uv(rc.slot),
									  rc.rotation });
		}
	}

	flush_batch(renderer, instance_batch, batch_texture, batch_slot);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	render_context_queue.clear();
	render_context_queue_needs_sort = false;
}

void Scene::add_texture_rc(const RenderContext &render_context) {
	Camera *camera = engine->get_camera();
	camera_dirty = camera_dirty ||
		cached_camera_position.x != camera->position.x ||
		cached_camera_position.y != camera->position.y ||
		cached_camera_zoom != camera->zoom;

	if (camera_dirty) {
		float view_width = camera->get_width();
		float view_height = camera->get_height();
		cached_camera_view =
			get_camera_view(camera->position.x, camera->position.y, camera->zoom,
				view_width, view_height);
		cached_camera_position = camera->position;
		cached_camera_zoom = camera->zoom;
		camera_dirty = false;
	}

	Rect object_aabb = compute_object_aabb_fast(render_context);

	if (!render_context.is_ui && !intersects(cached_camera_view, object_aabb)) {
		return;
	}

	if (!render_context_queue.empty()) {
		const RenderContext &last = render_context_queue.back();

		if (last.render_type != RT_CLEAR) {
			render_context_queue_needs_sort =
				render_context_queue_needs_sort ||
				render_context_less(render_context, last);
		}
	}

	render_context_queue.push_back(render_context);
}

void Scene::add_clear_rc(const RenderContext &render_context) {
	render_context_queue.push_back(render_context);
}

void Scene::add_render_context(const RenderContext &render_context) {
	switch (render_context.render_type) {
	case RT_TEXTURE:
		add_texture_rc(render_context);
		break;

	case RT_CLEAR:
		add_clear_rc(render_context);
		break;

	case RT_SHAPE:
		if (!render_context_queue.empty() &&
			render_context_queue.back().render_type != RT_CLEAR) {
			render_context_queue_needs_sort =
				render_context_queue_needs_sort ||
				render_context_less(render_context,
					render_context_queue.back());
		}
		render_context_queue.push_back(render_context);
		break;

	case RT_FRAMEBUFFER:
		if (!render_context_queue.empty() &&
			render_context_queue.back().render_type != RT_CLEAR) {
			render_context_queue_needs_sort =
				render_context_queue_needs_sort ||
				render_context_less(render_context,
					render_context_queue.back());
		}
		render_context_queue.push_back(render_context);
		break;

	case RT_MESH:
		if (!render_context_queue.empty() &&
			render_context_queue.back().render_type != RT_CLEAR) {
			render_context_queue_needs_sort =
				render_context_queue_needs_sort ||
				render_context_less(render_context,
					render_context_queue.back());
		}
		render_context_queue.push_back(render_context);
		break;
	}
}

void Scene::start_frame() { camera_dirty = true; }
