#include "renderer.h"
#include "camera.h"
#include "engine.h"
#include "framebuffer.h"
#include "render_context.h"
#include "scene.h"
#include "mathf.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "ttfont.h"

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>

Renderer::Renderer(Engine *engine, Camera *camera, Shader *shader)
	: engine(engine), camera(camera), shader(shader), default_shader(shader) {
	// Renderer builds shared GPU helpers used by every draw path: a white
	// texture for primitives, shape shader, framebuffer shader, and quad VAO.
	unsigned char white[] = { 255, 255, 255, 255 };
	white_texture = new Texture(1, 1, white);
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
}

void Renderer::set_shader(Shader *shader) { this->shader = shader; }

void Renderer::set_sprite_matrices(const glm::mat4 &view,
	const glm::mat4 &projection) {
	sprite_view = view;
	sprite_projection = projection;
}

const glm::mat4 &Renderer::get_sprite_view() const { return sprite_view; }
const glm::mat4 &Renderer::get_sprite_projection() const {
	return sprite_projection;
}

void Renderer::begin_frame() { clear_ui_masks(); }

void Renderer::push_ui_mask(const Vector &position, float width,
	float height) {
	const float half_width = std::abs(width) * 0.5f;
	const float half_height = std::abs(height) * 0.5f;
	glm::vec4 rect(position.x - half_width, position.y - half_height,
		position.x + half_width, position.y + half_height);

	if (!ui_mask_stack.empty()) {
		const glm::vec4 &parent = ui_mask_stack.back();
		rect.x = std::max(rect.x, parent.x);
		rect.y = std::max(rect.y, parent.y);
		rect.z = std::min(rect.z, parent.z);
		rect.w = std::min(rect.w, parent.w);
	}
	ui_mask_stack.push_back(rect);
}

void Renderer::pop_ui_mask() {
	if (!ui_mask_stack.empty())
		ui_mask_stack.pop_back();
}

void Renderer::clear_ui_masks() { ui_mask_stack.clear(); }

bool Renderer::has_ui_mask() const { return !ui_mask_stack.empty(); }

void Renderer::apply_ui_mask(RenderContext &context, bool is_ui) const {
	if (!is_ui || ui_mask_stack.empty())
		return;
	context.mask_enabled = true;
	context.mask_rect = ui_mask_stack.back();
}

void Renderer::upload_ui_mask(Shader *shader,
	const RenderContext &context) const {
	if (shader == nullptr)
		return;
	shader->upload_int("uMaskEnabled", context.mask_enabled ? 1 : 0);
	if (context.mask_enabled)
		shader->upload_vec4("uMaskRect", context.mask_rect);
}

void Renderer::render_texture(Texture *texture,
	Shader *shader, const Vector &position, float width,
	float height) {
	render_texture(texture, shader, position, width, height, 0.0f, color,
		texture == nullptr ? -1 : texture->get_atlas_slot(), false);
}

void Renderer::render_texture(Texture *texture, Shader *shader,
	const Vector &position, float width, float height, float rotation,
	const Color &color, int atlas_slot, bool is_ui) {
	// Public draw calls do not render immediately. They enqueue a command so
	// Scene can sort by z and batch compatible sprites later in the frame.
	if (texture == nullptr || shader == nullptr)
		return;
	FrameBuffer *framebuffer = this->engine->get_framebuffer();

	RenderContext rc = gen_render_context(framebuffer, texture, shader,
		position, atlas_slot, -rotation, width, height,
		color, is_ui);
	apply_ui_mask(rc, is_ui);
	engine->get_current_scene()->add_render_context(rc);
}

void Renderer::render_image(Texture *texture, const Vector &position,
	float width, float height) {
	Shader *current = Shader::get_current();
	render_texture(texture, current == nullptr ? default_shader : current,
		position, width, height, 0.0f, Color::white(),
		texture == nullptr ? -1 : texture->get_atlas_slot(), false);
}

void Renderer::render_image(Texture *texture, const Vector &position,
	float width, float height, float rotation) {
	Shader *current = Shader::get_current();
	render_texture(texture, current == nullptr ? default_shader : current,
		position, width, height, rotation, Color::white(),
		texture == nullptr ? -1 : texture->get_atlas_slot(), false);
}

void Renderer::render_ui_image(Texture *texture, const Vector &position,
	float width, float height) {
	Shader *current = Shader::get_current();
	render_texture(texture, current == nullptr ? default_shader : current,
		position, width, height, 0.0f, Color::white(),
		texture == nullptr ? -1 : texture->get_atlas_slot(), true);
}

void Renderer::render_ui_image(Texture *texture, const Vector &position,
	float width, float height, const Color &color) {
	Shader *current = Shader::get_current();
	render_texture(texture, current == nullptr ? default_shader : current,
		position, width, height, 0.0f, color,
		texture == nullptr ? -1 : texture->get_atlas_slot(), true);
}

void Renderer::render_ui_image(Texture *texture, const Vector &position,
	float width, float height, float rotation) {
	Shader *current = Shader::get_current();
	render_texture(texture, current == nullptr ? default_shader : current,
		position, width, height, rotation, Color::white(),
		texture == nullptr ? -1 : texture->get_atlas_slot(), true);
}

void Renderer::queue_shape(ShapeType shape_type, const Vector &position,
	float width, float height, float rotation, bool is_ui) {
	RenderContext rc = gen_shape_render_context(engine->get_framebuffer(),
		shape_type, position, -rotation, width, height, color, is_ui);
	apply_ui_mask(rc, is_ui);
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
	RenderContext rc = gen_framebuffer_render_context(engine->get_framebuffer(),
		source, position, -rotation, width, height, color, is_ui);
	rc.uv = uv;
	apply_ui_mask(rc, is_ui);
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
	RenderContext rc = gen_mesh_render_context(engine->get_framebuffer(), mesh,
		shader, position, -rotation, width, height, color, false);
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
	RenderContext rc = gen_mesh_render_context(engine->get_framebuffer(), mesh,
		shader, position, -rotation, width, height, color, true);
	apply_ui_mask(rc, true);
	engine->get_current_scene()->add_render_context(rc);
}

void Renderer::queue_font(TTFont *font, const std::string &text,
	const Vector &position, const std::string &align,
	const Color &outline_color, float outline_width, float rotation, bool is_ui) {
	if (font == nullptr || text.empty())
		return;

	Texture *texture = font->get_cached_texture(text, color, outline_color,
		outline_width);
	if (texture == nullptr)
		return;

	float width = static_cast<float>(texture->get_width());
	float height = static_cast<float>(texture->get_height());
	Vector render_position = position;
	if (align == "left")
		render_position.x += width * 0.5f;
	else if (align == "right")
		render_position.x -= width * 0.5f;

	Shader *current = Shader::get_current();
	render_texture(texture, current == nullptr ? default_shader : current,
		render_position, width, height, rotation,
		Color::white(), -1, is_ui);
}

void Renderer::render_font(TTFont *font, const std::string &text,
	const Vector &position, const std::string &align) {
	queue_font(font, text, position, align,
		Color::transparent(),
		0.0f, 0.0f, false);
}

void Renderer::render_font(TTFont *font, const std::string &text,
	const Vector &position, float rotation) {
	queue_font(font, text, position, "center",
		Color::transparent(),
		0.0f, rotation, false);
}

void Renderer::render_font(TTFont *font, const std::string &text,
	const Vector &position, const std::string &align,
	const Color &outline_color, float outline_width) {
	queue_font(font, text, position, align, outline_color, outline_width, 0.0f,
		false);
}

void Renderer::render_font(TTFont *font, const std::string &text,
	const Vector &position, float rotation, const std::string &align,
	const Color &outline_color, float outline_width) {
	queue_font(font, text, position, align, outline_color, outline_width, rotation,
		false);
}
void Renderer::render_ui_font(TTFont *font, const std::string &text,
	const Vector &position, const std::string &align) {
	queue_font(font, text, position, align, Color::outline(),
		0.0f, 0.0f, true);
}

void Renderer::render_ui_font(TTFont *font, const std::string &text,
	const Vector &position, const std::string &align,
	const Color &outline_color, float outline_width) {
	queue_font(font, text, position, align, outline_color, outline_width, 0.0f,
		true);
}

void Renderer::render_ui_font(TTFont *font, const std::string &text,
	const Vector &position, float rotation, const std::string &align) {
	queue_font(font, text, position, align,
		Color::transparent(),
		0.0f, rotation, true);
}

void Renderer::render_ui_font(TTFont *font, const std::string &text,
	const Vector &position, float rotation, const std::string &align,
	const Color &outline_color, float outline_width) {
	queue_font(font, text, position, align, outline_color, outline_width, rotation,
		true);
}


void Renderer::clear() {
	// Clears are immediate GL work, but first flush queued objects so commands
	// submitted before the clear really appear before it.
	// Toffee flushes queued objects before a clear so submission ordering is
	// preserved even when clear is called in the middle of Scene::render().
	if (this->engine->get_current_scene() != nullptr)
		this->engine->get_current_scene()->flush_render_context();
	FrameBuffer *framebuffer = this->engine->get_framebuffer();

	if (framebuffer != nullptr) {
		framebuffer->bind();
	}
	else {
		FrameBuffer::bind_screen_framebuffer();
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::apply_pixel_perfect_camera(FrameBuffer *framebuffer) {
	if (framebuffer == nullptr || !framebuffer->has_resolve_target())
		return;
	FrameBuffer *previous = FrameBuffer::get_bound_framebuffer();
	framebuffer->bind_resolve_target();
	RenderContext context = gen_framebuffer_render_context(nullptr, framebuffer,
		Vector(0.0f, 0.0f), 0.0f, 2.0f, 2.0f, Color::white(), true);
	context.is_pixel_perfect_framebuffer = true;
	const GLboolean blend_enabled = glIsEnabled(GL_BLEND);
	glDisable(GL_BLEND);
	draw_raw_texture(framebuffer->get_draw_color_texture(), context, glm::mat4(1.0f));
	if (blend_enabled) glEnable(GL_BLEND);
	if (previous != nullptr) previous->bind(); else FrameBuffer::bind_screen_framebuffer();
}

void Renderer::clear(float r, float g, float b, float a) {
	glClearColor(r, g, b, a);
	clear();
}

void Renderer::clear(const Color &color) {
	clear(color.r, color.g, color.b, color.a);
}

void Renderer::set_color(const Color &color) { this->color = color; }

Color Renderer::get_color() const { return color; }

Camera *Renderer::get_camera() { return camera; }

Shader *Renderer::get_shader() { return shader; }

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
	Shader *shape_shader = this->engine->get_shader_manager()->get_shader("shape");
	shape_shader->bind();
	shape_shader->upload_mat4("uViewProjection", vp);
	shape_shader->upload_vec4("uTransform",
		glm::vec4(rc.position.x, rc.position.y, rc.width, rc.height));
	shape_shader->upload_float("uRotation", rc.rotation);
	shape_shader->upload_color("uColor", rc.color);
	shape_shader->upload_int("uShapeType", static_cast<int>(rc.shape_type));
	upload_ui_mask(shape_shader, rc);

	glBindVertexArray(quad_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Renderer::draw_raw_texture(unsigned texture_id, const RenderContext &rc,
	const glm::mat4 &vp) {
	// Shared textured-quad path for sprites and framebuffer composites.
	if (texture_id == 0)
		return;

	Shader *quad_shader = this->engine->get_shader_manager()->get_shader("quad");
	quad_shader->bind();
	quad_shader->upload_mat4("uViewProjection", vp);
	quad_shader->upload_vec4("uTransform",
		glm::vec4(rc.position.x, rc.position.y, rc.width, rc.height));
	quad_shader->upload_vec4("uUv", rc.uv);
	quad_shader->upload_float("uRotation", rc.rotation);
	quad_shader->upload_color("uColor", rc.color);
	quad_shader->upload_int("uTexture", 0);
	quad_shader->upload_int("uPixelPerfect", rc.is_pixel_perfect_framebuffer ? 1 : 0);
	upload_ui_mask(quad_shader, rc);
	if (rc.is_pixel_perfect_framebuffer) {
		quad_shader->upload_vec2("uSourceSize", glm::vec2(rc.source_width, rc.source_height));
		quad_shader->upload_vec2("uOutputSize", glm::vec2(rc.pixel_output_width, rc.pixel_output_height));
		quad_shader->upload_vec2("uCameraOffset", camera->get_pixel_fraction(rc.pixel_per_unit));
		quad_shader->upload_float("uCameraZoom", camera->zoom);
		quad_shader->upload_float("uCameraRotation", camera->rotation);
		quad_shader->upload_float("uWorldUnitsPerPixel", rc.pixel_per_unit);
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glBindVertexArray(quad_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Renderer::draw_mesh(const RenderContext &rc, const glm::mat4 &vp) {
	// Mesh commands use a regular model matrix instead of sprite instancing.
	// Position, rotation, and size are still stored in RenderContext.
	if (rc.mesh == nullptr || rc.shader == nullptr)
		return;

	glm::mat4 model(1.0f);
	model = glm::translate(model,
		glm::vec3(rc.position.x, rc.position.y, rc.position.z));
	model = glm::rotate(model, rc.rotation, glm::vec3(0, 0, 1));
	model = glm::scale(model, glm::vec3(rc.width, rc.height, 1.0f));

	rc.shader->bind();
	rc.shader->upload_mat4("uView", glm::mat4(1.0f));
	rc.shader->upload_mat4("uProjection", vp);
	rc.shader->upload_mat4("uModel", model);
	rc.shader->upload_color("uColor", rc.color);
	upload_ui_mask(rc.shader, rc);

	glBindVertexArray(rc.mesh->get_vao());
	glDrawElements(rc.mesh->get_draw_mode(), rc.mesh->get_index_count(),
		GL_UNSIGNED_INT, 0);
}
