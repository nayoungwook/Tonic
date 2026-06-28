#pragma once

#include "engine/color.h"
#include <glm/glm.hpp>
#include "engine/mathf.h"

class Vector;
class Shader;
class Texture;
class FrameBuffer;
class Mesh;

enum RenderType {
	RT_TEXTURE = 0,
	RT_CLEAR = 1,
	RT_SHAPE = 2,
	RT_FRAMEBUFFER = 3,
	RT_MESH = 4,
};

enum ShapeType {
	SHAPE_RECT = 0,
	SHAPE_OVAL = 1,
};

class RenderContext {
public:
	enum RenderType render_type;
	FrameBuffer *framebuffer;
	Texture *texture;
	Mesh *mesh;
	Shader *shader;
	Vector position;

	Color color;
	glm::vec4 uv;

	int slot;
	unsigned raw_texture;
	float width, height;
	float source_width, source_height;
	float pixel_output_width, pixel_output_height;
	float rotation;
	int blend_source;
	int blend_destination;
	bool is_ui;
	ShapeType shape_type;

	bool is_pixel_perfect_framebuffer;
	float pixel_per_unit;
};

RenderContext gen_render_context(FrameBuffer *framebuffer, Texture *texture,
	Shader *shader, const Vector &position, int slot,
	float rotation, float width, float height,
	const Color &color = Color::white(),
	bool is_ui = false);
RenderContext gen_clear_render_context(FrameBuffer *framebuffer);
RenderContext gen_shape_render_context(FrameBuffer *framebuffer,
	ShapeType shape_type, const Vector &position, float rotation,
	float width, float height, const Color &color,
	bool is_ui = false);
RenderContext gen_framebuffer_render_context(FrameBuffer *target_framebuffer,
	FrameBuffer *source_framebuffer, const Vector &position, float rotation,
	float width, float height, const Color &color,
	bool is_ui = false);
RenderContext gen_mesh_render_context(FrameBuffer *framebuffer, Mesh *mesh,
	Shader *shader, const Vector &position, float rotation, float width,
	float height, const Color &color, bool is_ui = false);
