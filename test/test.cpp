#include <iostream>

#include "engine/engine.h"
#include "engine/framebuffer.h"

class Game : public Scene {
private:
	Texture *texture = nullptr;
	Shader *default_unlit = nullptr;
	Renderer *renderer = nullptr;
	Input *input = nullptr;
	Camera *camera = nullptr;
	TTFont *font = nullptr;

	Vector camera_target_position;
	FrameBuffer *ui_framebuffer = nullptr;
	FrameBuffer *game_framebuffer = nullptr;

	Sound *music = nullptr;

	const int MS = 60;
	float rotation = 0;
	float zoom = 1;
	float camera_move_speed = 10.5f;
	float camera_target_rotation = 0.0f;

public:
	Game(Engine *engine) : Scene(engine) {}

	void init() {
		texture = new Texture("resources/tile.png");
		ShaderManager *shader_manager =
			this->engine->get_shader_manager();
		default_unlit = shader_manager->get_shader("default_unlit");
		renderer = this->engine->get_renderer();
		input = this->engine->get_input();
		camera = this->engine->get_camera();
		font = new TTFont("resources/GalmuriMono11.ttf", 64);

		camera_target_position = Vector(0, 0);

		ui_framebuffer = new FrameBuffer();
		game_framebuffer = new FrameBuffer(true);
		game_framebuffer->set_pixel_per_unit(32, MS);
	}

	void update() {
		//rotation += 0.01f;

		if (input->is_key_down(SDL_SCANCODE_W)) {
			camera_target_position += Vector(0, 1) * camera_move_speed;
		}
		if (input->is_key_down(SDL_SCANCODE_S)) {
			camera_target_position -= Vector(0, 1) * camera_move_speed;
		}
		if (input->is_key_down(SDL_SCANCODE_A)) {
			camera_target_position -= Vector(1, 0) * camera_move_speed;
		}
		if (input->is_key_down(SDL_SCANCODE_D)) {
			camera_target_position += Vector(1, 0) * camera_move_speed;
		}

		if (input->is_key_down(SDL_SCANCODE_Q)) {
			camera_target_rotation -= 0.1f;
		}
		if (input->is_key_down(SDL_SCANCODE_E)) {
			camera_target_rotation += 0.1f;
		}

		this->camera->rotation += (camera_target_rotation - this->camera->rotation) * 0.1f;

		if (input->is_key_pressed(SDL_SCANCODE_F)) {
			this->engine->get_display()->set_fullscreen();
			//this->engine->get_display()->set_windowed(1920, 1080);
		}

		zoom += 0.1f * input->get_mouse_wheel_y();

		camera->zoom += (zoom - camera->zoom) / 5;
		camera->position += (camera_target_position - camera->position) / 5;
	}

	void render() {
		this->default_unlit->bind();

		this->engine->set_framebuffer(ui_framebuffer);
		renderer->clear(0.0f, 0.0f, 0.0f, 0.0f);
		renderer->set_color(Color(1, 1, 0, 1));
		renderer->render_ui_font(font, "this is for test.", Vector(0, 0, 1), rotation);


		this->engine->set_framebuffer(game_framebuffer);
		renderer->clear(0.05f, 0.05f, 0.07f, 1.0f);

		Vector mouse_world_pos = screen_to_world(this->camera, Vector(input->get_mouse_x(), input->get_mouse_y()));
		mouse_world_pos.z = 3;

		renderer->set_color(Color(1, 0, 0, 1));
		renderer->render_rect(mouse_world_pos, MS, MS, rotation);

		renderer->set_color(Color::white());
		int width = 200, height = 200;
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				renderer->render_image(texture, Vector((-width * 0.5f + j) * MS, (-height * 0.5f + i) * MS, 0), MS, MS, rotation);
			}
		}

		this->engine->set_framebuffer(nullptr);
		renderer->clear(0.0f, 0.0f, 0.0f, 1.0f);
		renderer->render_ui_framebuffer(game_framebuffer, Vector(0, 0, 10), camera->get_width(), camera->get_height());
		renderer->render_ui_framebuffer(ui_framebuffer, Vector(0, 0, 11), camera->get_width(), camera->get_height());
	}
};

int main(int argc, char *argv[]) {
	int width = 1280, height = 720;

	Engine *engine = new Engine();
	engine->init_engine("title", width, height);

	engine->set_scene(new Game(engine));

	engine->start();

	return 0;
}
