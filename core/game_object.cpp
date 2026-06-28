#include "engine/game_object.h"
#include "engine/engine.h"
#include "engine/error.h"
#include "engine/mesh.h"
#include "engine/render_context.h"
#include "engine/renderer.h"
#include "engine/scene.h"
#include "engine/shader_manager.h"
#include "engine/texture.h"

#include <GL/glew.h>

GameObject::GameObject(Engine *engine, const Vector &position, float width,
	float height)
	: engine(engine), position(position), width(width), height(height) {
	this->init();

	this->position.z = 1;
	this->slot = 0;
	this->texture = nullptr;
	this->shader = nullptr;
	this->blend_source = GL_SRC_ALPHA;
	this->blend_destination = GL_ONE_MINUS_SRC_ALPHA;
}

GameObject::GameObject(Engine *engine, float x, float y, float width,
	float height)
	: GameObject(engine, Vector(x, y), width, height) {}

void GameObject::render() {
	Scene *current_scene = this->engine->get_current_scene();
	if (current_scene == nullptr) {
		error("current scene is null");
	}

	Shader *render_shader = shader;
	if (render_shader == nullptr) {
		render_shader = mesh != nullptr
			? this->engine->get_shader_manager()->get_shader("default")
			: this->engine->get_renderer()->get_shader();
	}
	Vector render_position(position.x + render_offset_x,
		position.y + render_offset_y, position.z);

	float render_width = width + render_padding_x;
	float render_height = height + render_padding_y;

	if (flip_x)
		render_width *= -1.0f;
	if (flip_y)
		render_height *= -1.0f;

	RenderContext rc = {};
	if (mesh != nullptr) {
		rc = gen_mesh_render_context(this->engine->get_frame_buffer(),
			mesh, render_shader, render_position, rotation,
			render_width, render_height, glm::vec4(1.0f), ui);
	}
	else {
		if (texture == nullptr) {
			error("GameObject has no texture or mesh");
			return;
		}
		rc = gen_render_context(this->engine->get_frame_buffer(),
			this->texture, render_shader, render_position, this->slot,
			this->rotation, render_width, render_height, glm::vec4(1.0f), ui);
	}

	rc.blend_source = blend_source;
	rc.blend_destination = blend_destination;
	current_scene->add_render_context(rc);
}

void GameObject::init() {}
void GameObject::update() {}

bool GameObject::can_batch() const {
	return batchable && mesh == nullptr &&
		blend_source == GL_SRC_ALPHA &&
		blend_destination == GL_ONE_MINUS_SRC_ALPHA;
}

void GameObject::set_render_offset(float x, float y) {
	render_offset_x = x;
	render_offset_y = y;
}

void GameObject::set_render_padding(float x, float y) {
	render_padding_x = x;
	render_padding_y = y;
}

void GameObject::set_blend_mode(int source, int destination) {
	blend_source = source;
	blend_destination = destination;
}

int GameObject::get_render_atlas_index() const { return slot; }

void GameObject::set_render_atlas_index(int atlas_index) {
	slot = atlas_index;
}

bool GameObject::is_ui() const { return ui; }

void GameObject::set_is_ui(bool is_ui) { ui = is_ui; }
