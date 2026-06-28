#pragma once

#include "engine/color.h"
#include "engine/render_context.h"

#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <vector>

class Vector;
class Texture;
class Mesh;
class Engine;
class FrameBuffer;
class Shader;
class Camera;
class TTFont;

class Renderer {
public:
	Camera *get_camera();

	Renderer(Engine *engine, Camera *camera, Shader *shader);
	~Renderer();

	void render_texture(Texture *texture,
		Shader *shader, const Vector &position, float width,
		float height);
	void render_texture(Texture *texture, Shader *shader,
		const Vector &position, float width, float height, float rotation,
		const Color &color, int atlas_slot = -1, bool is_ui = false);
	void render_image(Texture *texture, const Vector &position, float width,
		float height);
	void render_image(Texture *texture, const Vector &position, float width,
		float height, float rotation);
	void render_ui_image(Texture *texture, const Vector &position,
		float width, float height);
	void render_ui_image(Texture *texture, const Vector &position,
		float width, float height, float rotation);
	void render_rect(const Vector &position, float width, float height);
	void render_rect(const Vector &position, float width, float height,
		float rotation);
	void render_ui_rect(const Vector &position, float width, float height);
	void render_ui_rect(const Vector &position, float width, float height,
		float rotation);
	void render_oval(const Vector &position, float width, float height);
	void render_oval(const Vector &position, float width, float height,
		float rotation);
	void render_ui_oval(const Vector &position, float width, float height);
	void render_ui_oval(const Vector &position, float width, float height,
		float rotation);
	void render_framebuffer(FrameBuffer *source, const Vector &position,
		float width, float height);
	void render_framebuffer(FrameBuffer *source, const Vector &position,
		float width, float height, float rotation);
	void render_ui_framebuffer(FrameBuffer *source, const Vector &position,
		float width, float height);
	void render_ui_framebuffer(FrameBuffer *source, const Vector &position,
		float width, float height, float rotation);
	void render_framebuffer_part(FrameBuffer *source, const Vector &position,
		float width, float height, const glm::vec4 &uv,
		bool is_ui = false);
	void render_framebuffer_part(FrameBuffer *source, const Vector &position,
		float width, float height, float rotation, const glm::vec4 &uv,
		bool is_ui = false);
	void render_mesh(Mesh *mesh, Shader *shader, const Vector &position,
		float width, float height);
	void render_mesh(Mesh *mesh, Shader *shader, const Vector &position,
		float width, float height, float rotation);
	void render_ui_mesh(Mesh *mesh, Shader *shader, const Vector &position,
		float width, float height);
	void render_ui_mesh(Mesh *mesh, Shader *shader, const Vector &position,
		float width, float height, float rotation);
	void render_font(TTFont *font, const std::string &text,
		const Vector &position, const std::string &align = "center");
	void render_font(TTFont *font, const std::string &text,
		const Vector &position, const std::string &align,
		const Color &outline_color, float outline_width);
	void render_font(TTFont *font, const std::string &text,
		const Vector &position, float rotation);
	void render_ui_font(TTFont *font, const std::string &text,
		const Vector &position, const std::string &align = "center");
	void render_ui_font(TTFont *font, const std::string &text,
		const Vector &position, const std::string &align,
		const Color &outline_color, float outline_width);
	void render_ui_font(TTFont *font, const std::string &text,
		const Vector &position, float rotation, const std::string &align = "center");
	void render_ui_font(TTFont *font, const std::string &text,
		const Vector &position, float rotation, const std::string &align,
		const Color &outline_color, float outline_width);

	void begin_frame();
	void clear();
	void clear(float r, float g, float b, float a = 1.0f);
	void clear(const Color &color);
	void set_color(const Color &color);
	Color get_color() const;

	Shader *get_shader();
	void set_shader(Shader *shader);
	Shader *get_shape_shader();
	Texture *get_white_texture();
	void draw_shape(const class RenderContext &rc, const glm::mat4 &vp);
	void draw_raw_texture(unsigned texture_id, const class RenderContext &rc,
		const glm::mat4 &vp);
	void draw_mesh(const class RenderContext &rc, const glm::mat4 &vp);

private:
	void init_quad();
	void queue_shape(ShapeType shape_type, const Vector &position,
		float width, float height, float rotation, bool is_ui);
	void queue_font(TTFont *font, const std::string &text,
		const Vector &position, const std::string &align,
		const Color &outline_color, float outline_width, float rotation, bool is_ui);

	Engine *engine;
	Shader *shader;
	Shader *shape_shader = nullptr;
	Shader *quad_shader = nullptr;
	Texture *white_texture = nullptr;
	Camera *camera;
	Color color = Color::white();
	std::vector<std::unique_ptr<Texture>> frame_textures;
	unsigned quad_vao = 0;
	unsigned quad_vbo = 0;
	unsigned quad_ebo = 0;
};
