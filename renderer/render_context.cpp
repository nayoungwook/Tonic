#include "engine/render_context.h"

#include "engine/framebuffer.h"

#include <GL/glew.h>
#include <iostream>

RenderContext gen_render_context(FrameBuffer *framebuffer, Texture *texture,
	Shader *shader, const Vector &position, int slot,
	float rotation, float width, float height,
	const Color &color,
	bool is_ui) {
	RenderContext result = {};

	result.framebuffer = framebuffer;
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
	result.source_width = 0.0f;
	result.source_height = 0.0f;
	result.pixel_output_width = width;
	result.pixel_output_height = height;
	result.blend_source = GL_SRC_ALPHA;
	result.blend_destination = GL_ONE_MINUS_SRC_ALPHA;
	result.slot = slot;
	result.raw_texture = 0;
	result.is_ui = is_ui;
	result.shape_type = SHAPE_RECT;
	result.is_pixel_perfect_framebuffer = false;
	result.pixel_per_unit = 1.0f;

	return result;
}

RenderContext gen_clear_render_context(FrameBuffer *framebuffer) {
	RenderContext result = {};

	result.render_type = RT_CLEAR;
	result.framebuffer = framebuffer;
	result.blend_source = GL_SRC_ALPHA;
	result.blend_destination = GL_ONE_MINUS_SRC_ALPHA;
	result.is_pixel_perfect_framebuffer = false;
	result.pixel_per_unit = 1.0f;
	result.source_width = 0.0f;
	result.source_height = 0.0f;
	result.pixel_output_width = 0.0f;
	result.pixel_output_height = 0.0f;

	return result;
}

RenderContext gen_shape_render_context(FrameBuffer *framebuffer,
	ShapeType shape_type,
	const Vector &position, float rotation,
	float width, float height,
	const Color &color,
	bool is_ui) {
	RenderContext result = {};

	result.render_type = RT_SHAPE;
	result.framebuffer = framebuffer;
	result.position = position;
	result.rotation = rotation;
	result.width = width;
	result.height = height;
	result.source_width = 0.0f;
	result.source_height = 0.0f;
	result.pixel_output_width = width;
	result.pixel_output_height = height;
	result.color = color;
	result.blend_source = GL_SRC_ALPHA;
	result.blend_destination = GL_ONE_MINUS_SRC_ALPHA;
	result.is_ui = is_ui;
	result.shape_type = shape_type;
	result.is_pixel_perfect_framebuffer = false;
	result.pixel_per_unit = 1.0f;

	return result;
}

RenderContext gen_mesh_render_context(FrameBuffer *framebuffer, Mesh *mesh,
	Shader *shader, const Vector &position,
	float rotation, float width,
	float height, const Color &color,
	bool is_ui) {
	RenderContext result = {};

	result.render_type = RT_MESH;
	result.framebuffer = framebuffer;
	result.mesh = mesh;
	result.shader = shader;
	result.position = position;
	result.rotation = rotation;
	result.width = width;
	result.height = height;
	result.source_width = 0.0f;
	result.source_height = 0.0f;
	result.pixel_output_width = width;
	result.pixel_output_height = height;
	result.color = color;
	result.blend_source = GL_SRC_ALPHA;
	result.blend_destination = GL_ONE_MINUS_SRC_ALPHA;
	result.is_ui = is_ui;
	result.is_pixel_perfect_framebuffer = false;
	result.pixel_per_unit = 1.0f;

	return result;
}

RenderContext gen_framebuffer_render_context(FrameBuffer *target_framebuffer,
	FrameBuffer *source_framebuffer,
	const Vector &position,
	float rotation, float width,
	float height,
	const Color &color,
	bool is_ui) {
	RenderContext result = {};

	result.render_type = RT_FRAMEBUFFER;
	result.framebuffer = target_framebuffer;
	result.position = position;
	result.rotation = rotation;
	result.width = width;
	result.height = height;
	result.source_width =
		source_framebuffer == nullptr ? 0.0f :
		static_cast<float>(source_framebuffer->get_width());
	result.source_height =
		source_framebuffer == nullptr ? 0.0f :
		static_cast<float>(source_framebuffer->get_height());
	result.pixel_output_width =
		source_framebuffer == nullptr ? width :
		static_cast<float>(source_framebuffer->get_pixel_view_width());
	result.pixel_output_height =
		source_framebuffer == nullptr ? height :
		static_cast<float>(source_framebuffer->get_pixel_view_height());
	result.color = color;
	result.blend_source = GL_SRC_ALPHA;
	result.blend_destination = GL_ONE_MINUS_SRC_ALPHA;
	result.is_ui = is_ui;
	result.uv = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	result.raw_texture =
		source_framebuffer == nullptr ? 0 : source_framebuffer->get_color_texture();

	result.is_pixel_perfect_framebuffer =
		source_framebuffer != nullptr && source_framebuffer->is_pixel_perfect();
	result.pixel_per_unit =
		source_framebuffer == nullptr ? 1.0f : source_framebuffer->get_pixel_per_unit();

	return result;
}
