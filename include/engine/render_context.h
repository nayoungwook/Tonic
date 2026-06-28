#pragma once

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
	FrameBuffer *frame_buffer;
	Texture *texture;
	Mesh *mesh;
	Shader *shader;
	Vector position;
	glm::vec4 color;
	glm::vec4 uv;
	int slot;
	unsigned raw_texture;
	float width, height;
	float rotation;
	int blend_source;
	int blend_destination;
	bool is_ui;
	ShapeType shape_type;
};

RenderContext gen_render_context(FrameBuffer *frame_buffer, Texture *texture,
	Shader *shader, const Vector &position, int slot,
	float rotation, float width, float height,
	const glm::vec4 &color = glm::vec4(1.0f),
	bool is_ui = false);
RenderContext gen_clear_render_context(FrameBuffer *frame_buffer);
RenderContext gen_shape_render_context(FrameBuffer *frame_buffer,
	ShapeType shape_type, const Vector &position, float rotation,
	float width, float height, const glm::vec4 &color,
	bool is_ui = false);
RenderContext gen_framebuffer_render_context(FrameBuffer *target_frame_buffer,
	FrameBuffer *source_frame_buffer, const Vector &position, float rotation,
	float width, float height, const glm::vec4 &color,
	bool is_ui = false);
RenderContext gen_mesh_render_context(FrameBuffer *frame_buffer, Mesh *mesh,
	Shader *shader, const Vector &position, float rotation, float width,
	float height, const glm::vec4 &color, bool is_ui = false);
