#pragma once

#include <vector>

#include "engine/render_context.h"
#include "engine/texture.h"
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

	void flush_batch(Renderer *renderer,
		std::vector<TextureInstance> &instance_batch,
		Texture *batch_texture, int &batch_slot);

	void start_frame();
	Engine *get_engine();

protected:
	Engine *engine;

private:
	Shader *shader_cache = nullptr;
	bool shader_cache_is_ui = false;
	bool shader_cache_pixel_perfect = false;
	FrameBuffer *frame_buffer_cache = nullptr;
	FrameBuffer *batch_frame_buffer = nullptr;
	Texture *batch_texture = nullptr;
	Shader *batch_shader = nullptr;
	int texture_uniform_location = -1;
	bool default_frame_buffer_bound = false;
	int view_projection_uniform_location = -1;
	int sprite_pixel_perfect_uniform_location = -1;
	int batch_slot = 0;
	bool batch_is_ui = false;
	bool batch_pixel_perfect = false;
	int batch_blend_source = GL_SRC_ALPHA;
	int batch_blend_destination = GL_ONE_MINUS_SRC_ALPHA;

	void add_texture_rc(const RenderContext &render_context);
	void add_clear_rc(const RenderContext &render_context);

	inline void Scene::bind_frame_buffer(FrameBuffer *frame_buffer);

	inline void rt_clear(Renderer *renderer, FrameBuffer *frame_buffer);
	inline void rt_shape(const RenderContext &rc, const glm::mat4 &view_projection, Renderer *renderer, FrameBuffer *frame_buffer);
	inline void rt_frame_buffer(const RenderContext &rc, const glm::mat4 &view_projection, Renderer *renderer, FrameBuffer *frame_buffer);
	inline void rt_mesh(const RenderContext &rc, const glm::mat4 &view_projection, Renderer *renderer, FrameBuffer *frame_buffer);

	Rect get_camera_view(float cam_x, float cam_y, float zoom, float screen_w,
		float screen_h);

	Rect compute_object_aabb_fast(const RenderContext &rc);

	bool intersects(const Rect &a, const Rect &b);

	std::vector<RenderContext> render_context_queue;
	std::vector<TextureInstance> instance_batch;
	bool render_context_queue_needs_sort = false;

	Rect cached_camera_view;
	Vector cached_camera_position;
	float cached_camera_zoom = 1.0f;
	bool camera_dirty = true;
};

void set_scene(Scene *scene);
