#include "engine/renderer.h"
#include "engine/camera.h"
#include "engine/engine.h"
#include "engine/framebuffer.h"
#include "engine/render_context.h"
#include "engine/scene.h"
#include "engine/mathf.h"
#include "engine/mesh.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/ttfont.h"

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>

Renderer::Renderer(Engine *engine, Camera *camera, Shader *shader)
	: engine(engine), camera(camera), shader(shader) {
	unsigned char white[] = { 255, 255, 255, 255 };
	white_texture = new Texture(1, 1, white);
	shape_shader = new Shader("shaders/shape.frag", "shaders/shape.vert");
	quad_shader = new Shader("shaders/quad.frag", "shaders/quad.vert");
	init_quad();
}

Renderer::~Renderer() {
	if (quad_ebo != 0)
		glDeleteBuffers(1, &quad_ebo);
	if (quad_vbo != 0)
		glDeleteBuffers(1, &quad_vbo);
	if (quad_vao != 0)
		glDeleteVertexArrays(1, &quad_vao);
	delete white_texture;
	delete shape_shader;
	delete quad_shader;
}

void Renderer::set_shader(Shader *shader) { this->shader = shader; }

void Renderer::begin_frame() {
	frame_textures.clear();
}

void Renderer::render_texture(Texture *texture,
	Shader *shader, const Vector &position, float width,
	float height) {
	render_texture(texture, shader, position, width, height, 0.0f, color,
		texture == nullptr ? -1 : texture->get_atlas_slot(), false);
}

void Renderer::render_texture(Texture *texture, Shader *shader,
	const Vector &position, float width, float height, float rotation,
	const glm::vec4 &color, int atlas_slot, bool is_ui) {
	if (texture == nullptr || shader == nullptr)
		return;
	FrameBuffer *frame_buffer = this->engine->get_frame_buffer();

	RenderContext rc = gen_render_context(frame_buffer, texture, shader,
		position, atlas_slot, rotation, width, height, color, is_ui);
	engine->get_current_scene()->add_render_context(rc);
}

void Renderer::render_image(Texture *texture, const Vector &position,
	float width, float height) {
	render_texture(texture, shader, position, width, height);
}

void Renderer::render_image(Texture *texture, const Vector &position,
	float width, float height, float rotation) {
	render_texture(texture, shader, position, width, height, rotation, color,
		texture == nullptr ? -1 : texture->get_atlas_slot(), false);
}

void Renderer::render_ui_image(Texture *texture, const Vector &position,
	float width, float height) {
	render_texture(texture, shader, position, width, height, 0.0f, color,
		texture == nullptr ? -1 : texture->get_atlas_slot(), true);
}

void Renderer::render_ui_image(Texture *texture, const Vector &position,
	float width, float height, float rotation) {
	render_texture(texture, shader, position, width, height, rotation, color,
		texture == nullptr ? -1 : texture->get_atlas_slot(), true);
}

void Renderer::queue_shape(ShapeType shape_type, const Vector &position,
	float width, float height, float rotation, bool is_ui) {
	RenderContext rc = gen_shape_render_context(engine->get_frame_buffer(),
		shape_type, position, rotation, width, height, color, is_ui);
	engine->get_current_scene()->add_render_context(rc);
}

void Renderer::render_rect(const Vector &position, float width, float height) {
	queue_shape(SHAPE_RECT, position, width, height, 0.0f, false);
}

void Renderer::render_rect(const Vector &position, float width, float height,
	float rotation) {
	queue_shape(SHAPE_RECT, position, width, height, rotation, false);
}

void Renderer::render_ui_rect(const Vector &position, float width,
	float height) {
	queue_shape(SHAPE_RECT, position, width, height, 0.0f, true);
}

void Renderer::render_ui_rect(const Vector &position, float width,
	float height, float rotation) {
	queue_shape(SHAPE_RECT, position, width, height, rotation, true);
}

void Renderer::render_oval(const Vector &position, float width, float height) {
	queue_shape(SHAPE_OVAL, position, width, height, 0.0f, false);
}

void Renderer::render_oval(const Vector &position, float width, float height,
	float rotation) {
	queue_shape(SHAPE_OVAL, position, width, height, rotation, false);
}

void Renderer::render_ui_oval(const Vector &position, float width,
	float height) {
	queue_shape(SHAPE_OVAL, position, width, height, 0.0f, true);
}

void Renderer::render_ui_oval(const Vector &position, float width,
	float height, float rotation) {
	queue_shape(SHAPE_OVAL, position, width, height, rotation, true);
}

void Renderer::render_framebuffer(FrameBuffer *source, const Vector &position,
	float width, float height) {
	render_framebuffer(source, position, width, height, 0.0f);
}

void Renderer::render_framebuffer(FrameBuffer *source, const Vector &position,
	float width, float height, float rotation) {
	render_framebuffer_part(source, position, width, height, rotation,
		glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), false);
}

void Renderer::render_ui_framebuffer(FrameBuffer *source, const Vector &position,
	float width, float height) {
	render_framebuffer_part(source, position, width, height, 0,
		glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), true);
}

void Renderer::render_ui_framebuffer(FrameBuffer *source, const Vector &position,
	float width, float height, float rotation) {
	render_framebuffer_part(source, position, width, height, rotation,
		glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), true);
}

void Renderer::render_framebuffer_part(FrameBuffer *source,
	const Vector &position, float width, float height, const glm::vec4 &uv,
	bool is_ui) {
	render_framebuffer_part(source, position, width, height, 0.0f, uv, is_ui);
}

void Renderer::render_framebuffer_part(FrameBuffer *source,
	const Vector &position, float width, float height, float rotation,
	const glm::vec4 &uv, bool is_ui) {
	if (source == nullptr)
		return;
	RenderContext rc = gen_framebuffer_render_context(engine->get_frame_buffer(),
		source, position, rotation, width, height, color, is_ui);
	rc.uv = uv;
	engine->get_current_scene()->add_render_context(rc);
}

void Renderer::render_mesh(Mesh *mesh, Shader *shader, const Vector &position,
	float width, float height) {
	render_mesh(mesh, shader, position, width, height, 0.0f);
}

void Renderer::render_mesh(Mesh *mesh, Shader *shader, const Vector &position,
	float width, float height, float rotation) {
	if (mesh == nullptr || shader == nullptr)
		return;
	RenderContext rc = gen_mesh_render_context(engine->get_frame_buffer(), mesh,
		shader, position, rotation, width, height, color, false);
	engine->get_current_scene()->add_render_context(rc);
}

void Renderer::render_ui_mesh(Mesh *mesh, Shader *shader,
	const Vector &position, float width, float height) {
	render_ui_mesh(mesh, shader, position, width, height, 0.0f);
}

void Renderer::render_ui_mesh(Mesh *mesh, Shader *shader,
	const Vector &position, float width, float height, float rotation) {
	if (mesh == nullptr || shader == nullptr)
		return;
	RenderContext rc = gen_mesh_render_context(engine->get_frame_buffer(), mesh,
		shader, position, rotation, width, height, color, true);
	engine->get_current_scene()->add_render_context(rc);
}

void Renderer::queue_font(TTFont *font, const std::string &text,
	const Vector &position, const std::string &align,
	const glm::vec4 &outline_color, float outline_width, float rotation, bool is_ui) {
	if (font == nullptr || text.empty())
		return;

	std::unique_ptr<Texture> text_texture =
		font->create_texture(text, color, outline_color, outline_width);
	if (!text_texture)
		return;

	float width = static_cast<float>(text_texture->get_width());
	float height = static_cast<float>(text_texture->get_height());
	Vector render_position = position;
	if (align == "left")
		render_position.x += width * 0.5f;
	else if (align == "right")
		render_position.x -= width * 0.5f;

	Texture *texture = text_texture.get();
	frame_textures.push_back(std::move(text_texture));
	render_texture(texture, shader, render_position, width, height, rotation,
		glm::vec4(1.0f), -1, is_ui);
}

void Renderer::render_font(TTFont *font, const std::string &text,
	const Vector &position, const std::string &align) {
	queue_font(font, text, position, align,
		glm::vec4(0, 0, 0, 0.0f),
		0.0f, 0.0f, false);
}

void Renderer::render_font(TTFont *font, const std::string &text,
	const Vector &position, float rotation) {
	queue_font(font, text, position, "center",
		glm::vec4(0, 0, 0, 0.0f),
		0.0f, rotation, false);
}

void Renderer::render_font(TTFont *font, const std::string &text,
	const Vector &position, const std::string &align,
	const glm::vec4 &outline_color, float outline_width) {
	queue_font(font, text, position, align, outline_color, outline_width, 0.0f,
		false);
}

void Renderer::render_ui_font(TTFont *font, const std::string &text,
	const Vector &position, const std::string &align) {
	queue_font(font, text, position, align,
		glm::vec4(39.0f / 255.0f, 39.0f / 255.0f, 54.0f / 255.0f, 1.0f),
		0.0f, 0.0f, true);
}

void Renderer::render_ui_font(TTFont *font, const std::string &text,
	const Vector &position, const std::string &align,
	const glm::vec4 &outline_color, float outline_width) {
	queue_font(font, text, position, align, outline_color, outline_width, 0.0f,
		true);
}

void Renderer::render_ui_font(TTFont *font, const std::string &text,
	const Vector &position, float rotation, const std::string &align) {
	queue_font(font, text, position, align,
		glm::vec4(0, 0, 0, 0),
		0.0f, rotation, true);
}

void Renderer::render_ui_font(TTFont *font, const std::string &text,
	const Vector &position, float rotation, const std::string &align,
	const glm::vec4 &outline_color, float outline_width) {
	queue_font(font, text, position, align, outline_color, outline_width, rotation,
		true);
}


void Renderer::clear() {
	FrameBuffer *frame_buffer = this->engine->get_frame_buffer();

	if (frame_buffer != nullptr) {
		frame_buffer->bind();
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::clear(float r, float g, float b, float a) {
	glClearColor(r, g, b, a);
	clear();
}

void Renderer::set_color(const glm::vec4 &color) { this->color = color; }

glm::vec4 Renderer::get_color() const { return color; }

Camera *Renderer::get_camera() { return camera; }

Shader *Renderer::get_shader() { return shader; }

Shader *Renderer::get_shape_shader() { return shape_shader; }

Texture *Renderer::get_white_texture() { return white_texture; }

void Renderer::init_quad() {
	if (quad_vao != 0)
		return;

	float vertices[] = {
		-0.5f, -0.5f, 0.0f, 0.0f,
		0.5f, -0.5f, 1.0f, 0.0f,
		0.5f, 0.5f, 1.0f, 1.0f,
		-0.5f, 0.5f, 0.0f, 1.0f,
	};
	unsigned indices[] = { 0, 1, 2, 2, 3, 0 };

	glGenVertexArrays(1, &quad_vao);
	glGenBuffers(1, &quad_vbo);
	glGenBuffers(1, &quad_ebo);

	glBindVertexArray(quad_vao);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
		GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
		(void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
		(void *)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

void Renderer::draw_shape(const RenderContext &rc, const glm::mat4 &vp) {
	shape_shader->bind();
	shape_shader->upload_mat4("uViewProjection", vp);
	shape_shader->upload_vec4("uTransform",
		glm::vec4(rc.position.x, rc.position.y, rc.width, rc.height));
	shape_shader->upload_float("uRotation", rc.rotation);
	shape_shader->upload_vec4("uColor", rc.color);
	shape_shader->upload_int("uShapeType", static_cast<int>(rc.shape_type));

	glBindVertexArray(quad_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Renderer::draw_raw_texture(unsigned texture_id, const RenderContext &rc,
	const glm::mat4 &vp) {
	if (texture_id == 0)
		return;
	quad_shader->bind();
	quad_shader->upload_mat4("uViewProjection", vp);
	quad_shader->upload_vec4("uTransform",
		glm::vec4(rc.position.x, rc.position.y, rc.width, rc.height));
	quad_shader->upload_vec4("uUv", rc.uv);
	quad_shader->upload_float("uRotation", rc.rotation);
	quad_shader->upload_vec4("uColor", rc.color);
	quad_shader->upload_int("uTexture", 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glBindVertexArray(quad_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Renderer::draw_mesh(const RenderContext &rc, const glm::mat4 &vp) {
	if (rc.mesh == nullptr || rc.shader == nullptr)
		return;

	glm::mat4 model(1.0f);
	model = glm::translate(model,
		glm::vec3(rc.position.x, rc.position.y, rc.position.z));
	model = glm::rotate(model, rc.rotation, glm::vec3(0, 0, 1));
	model = glm::scale(model, glm::vec3(rc.width, rc.height, 1.0f));

	rc.shader->bind();
	rc.shader->upload_mat4("uViewProjection", vp);
	rc.shader->upload_mat4("uModel", model);
	rc.shader->upload_vec4("uColor", rc.color);

	glBindVertexArray(rc.mesh->get_vao());
	glDrawElements(rc.mesh->get_draw_mode(), rc.mesh->get_index_count(),
		GL_UNSIGNED_INT, 0);
}
