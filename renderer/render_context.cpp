#include "engine/render_context.h"

#include "engine/framebuffer.h"

#include <GL/glew.h>

RenderContext gen_render_context(FrameBuffer *frame_buffer, Texture *texture,
	Shader *shader, const Vector &position, int slot,
	float rotation, float width, float height,
	const glm::vec4 &color,
	bool is_ui) {
	RenderContext result = {};

	result.frame_buffer = frame_buffer;
	result.render_type = RT_TEXTURE;
	result.texture = texture;
	result.mesh = nullptr;
	result.shader = shader;
	result.position = position;
	result.color = color;
	result.uv = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	result.rotation = rotation;
	result.width = width;
	result.height = height;
	result.position.z = position.z;
	result.blend_source = GL_SRC_ALPHA;
	result.blend_destination = GL_ONE_MINUS_SRC_ALPHA;
	result.slot = slot;
	result.raw_texture = 0;
	result.is_ui = is_ui;
	result.shape_type = SHAPE_RECT;

	return result;
}

RenderContext gen_clear_render_context(FrameBuffer *frame_buffer) {
	RenderContext result = {};

	result.render_type = RT_CLEAR;
	result.frame_buffer = frame_buffer;
	result.blend_source = GL_SRC_ALPHA;
	result.blend_destination = GL_ONE_MINUS_SRC_ALPHA;

	return result;
}

RenderContext gen_shape_render_context(FrameBuffer *frame_buffer,
	ShapeType shape_type,
	const Vector &position, float rotation,
	float width, float height,
	const glm::vec4 &color,
	bool is_ui) {
	RenderContext result = {};

	result.render_type = RT_SHAPE;
	result.frame_buffer = frame_buffer;
	result.position = position;
	result.rotation = rotation;
	result.width = width;
	result.height = height;
	result.color = color;
	result.position.z = position.z;
	result.blend_source = GL_SRC_ALPHA;
	result.blend_destination = GL_ONE_MINUS_SRC_ALPHA;
	result.is_ui = is_ui;
	result.shape_type = shape_type;

	return result;
}

RenderContext gen_mesh_render_context(FrameBuffer *frame_buffer, Mesh *mesh,
	Shader *shader, const Vector &position,
	float rotation, float width,
	float height, const glm::vec4 &color,
	bool is_ui) {
	RenderContext result = {};

	result.render_type = RT_MESH;
	result.frame_buffer = frame_buffer;
	result.mesh = mesh;
	result.shader = shader;
	result.position = position;
	result.rotation = rotation;
	result.width = width;
	result.height = height;
	result.color = color;
	result.position.z = position.z;
	result.blend_source = GL_SRC_ALPHA;
	result.blend_destination = GL_ONE_MINUS_SRC_ALPHA;
	result.is_ui = is_ui;

	return result;
}

RenderContext gen_framebuffer_render_context(FrameBuffer *target_frame_buffer,
	FrameBuffer *source_frame_buffer,
	const Vector &position,
	float rotation, float width,
	float height,
	const glm::vec4 &color,
	bool is_ui) {
	RenderContext result = {};

	result.render_type = RT_FRAMEBUFFER;
	result.frame_buffer = target_frame_buffer;
	result.position = position;
	result.rotation = rotation;
	result.width = width;
	result.height = height;
	result.color = color;
	result.position.z = position.z;
	result.blend_source = GL_SRC_ALPHA;
	result.blend_destination = GL_ONE_MINUS_SRC_ALPHA;
	result.is_ui = is_ui;
	result.uv = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	result.raw_texture =
		source_frame_buffer == nullptr ? 0 : source_frame_buffer->get_color_texture();

	return result;
}
