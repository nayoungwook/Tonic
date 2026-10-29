/*
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include <ui_button.h>
#include <ui_scroll.h>
#include <engine.h>
#include <framebuffer.h>
#include <game_object.h>

class TestObject : public GameObject {
public:
	TestObject(Engine *engine, const Vector &position) : GameObject(engine, position, 60, 60) {}

	void update() {
		this->rotation += 0.01f;
	}
};

class Game : public Scene {
private:
	Texture *texture = nullptr;
	Texture *button = nullptr;
	Shader *sprite_shader = nullptr;
	Renderer *renderer = nullptr;
	Input *input = nullptr;
	Camera *camera = nullptr;
	TTFont *font = nullptr;

	Sound *music = nullptr;
	Sound *click = nullptr;

	TestObject *test_obj = nullptr;

	Vector camera_target_position;
	const int MS = 60;
	float rotation = 0;
	float zoom = 1.0f;
	float camera_move_speed = 10.5f;
	float camera_target_rotation = 0.0f;

	FrameBuffer *game_framebuffer;
	FrameBuffer *ui_framebuffer;

	UIButton *ui_button = nullptr;
	UIScroll item_scroll = UIScroll(Vector(0.0f, -245.0f, 2.0f),
		780.0f, 150.0f, UIScrollDirection::Horizontal);
	std::vector<UIButton> scroll_buttons;
	int selected_item = -1;
	static constexpr int SCROLL_ITEM_COUNT = 10;
	static constexpr float SCROLL_ITEM_EXTENT = 100.0f;
	static constexpr float SCROLL_ITEM_SPACING = 16.0f;
	Shader *blend_shader = nullptr;

public:
	Game(Engine *engine) : Scene(engine) {}

	void init() {
		texture = new Texture("resources/tile.png");
		button = new Texture("resources/aru.png");

		music = new Sound("resources/ChickenGame.mp3",
			SoundPlaybackMode::Single);

		click = new Sound("resources/click.mp3",
			SoundPlaybackMode::Overlap, 8);

		ShaderManager *shader_manager =
			this->engine->get_shader_manager();
		sprite_shader = shader_manager->get_shader("sprite");

		ui_button = new UIButton(button, Vector(0.0f, 80.0f, 3.0f),
			140.0f, 170.0f);
		ui_button->set_render_size(90.0f, 150.0f);
		ui_button->set_tints(Color::white(),
			Color(1.0f, 0.82f, 0.82f, 1.0f),
			Color(0.75f, 0.60f, 0.60f, 1.0f),
			Color(0.45f, 0.45f, 0.45f, 0.75f));
		ui_button->set_on_click([this]() {
			std::cout << "main button clicked\n";
			click->play();
		});

		item_scroll.set_wheel_step(SCROLL_ITEM_EXTENT +
			SCROLL_ITEM_SPACING);
		item_scroll.set_smoothing(0.22f);
		scroll_buttons.reserve(SCROLL_ITEM_COUNT);
		for (int i = 0; i < SCROLL_ITEM_COUNT; ++i) {
			scroll_buttons.emplace_back(button, Vector(),
				SCROLL_ITEM_EXTENT, 130.0f);
			UIButton &item = scroll_buttons.back();
			item.set_render_size(62.0f, 108.0f);
			item.set_tints(Color::white(),
				Color(1.0f, 0.90f, 0.70f, 1.0f),
				Color(0.75f, 0.65f, 0.45f, 1.0f),
				Color(0.45f, 0.45f, 0.45f, 0.75f));
			item.set_on_click([this, i]() {
				selected_item = i;
				std::cout << "scroll item " << i + 1 << " clicked\n";
				click->play();
			});
		}

		renderer = this->engine->get_renderer();
		input = this->engine->get_input();
		camera = this->engine->get_camera();
		font = new TTFont("resources/GalmuriMono11.ttf", 32);

		game_framebuffer = new FrameBuffer(FrameBuffer::SizeMode::RENDER, true);
		game_framebuffer->set_pixel_per_unit(16.0f, MS);

		ui_framebuffer = new FrameBuffer(FrameBuffer::SizeMode::CAMERA);

		camera_target_position = Vector(0, 0);
		blend_shader = new Shader("resources/blend.vert", "resources/blend.frag");

		this->engine->set_automatic_presentation(false);

		test_obj = new TestObject(this->engine, Vector(0, 0));
		test_obj->texture = texture;
	}

	void update() {
		rotation += 0.01f;

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
			music->set_volume(music->get_volume() + 0.01f);
		}
		if (input->is_key_down(SDL_SCANCODE_E)) {
			camera_target_rotation += 0.1f;
			music->set_volume(music->get_volume() - 0.01f);
		}

		if (input->is_key_pressed(SDL_SCANCODE_SPACE)) {
			if (music->is_playing()) {
				music->pause();
			}
			else if (music->is_paused()) {
				music->resume();
			}
			else {
				music->play();
			}
		}
		if (input->is_key_pressed(SDL_SCANCODE_C)) {
			for (int i = 0; i < 4; ++i)
				click->play();
		}

		this->camera->rotation += (camera_target_rotation - this->camera->rotation) * 0.1f;

		this->test_obj->update();

		if (input->is_key_pressed(SDL_SCANCODE_F)) {
			this->engine->get_display()->set_fullscreen();
		}

		ui_button->update(input);

		const bool scroll_used_wheel = item_scroll.update(input,
			static_cast<int>(scroll_buttons.size()), SCROLL_ITEM_EXTENT,
			SCROLL_ITEM_SPACING);
		const bool mouse_inside_scroll = item_scroll.contains(
			input->get_ui_mouse_x(), input->get_ui_mouse_y());
		for (int i = 0; i < static_cast<int>(scroll_buttons.size()); ++i) {
			UIButton &item = scroll_buttons[i];
			Vector item_position = item_scroll.get_item_position(i,
				static_cast<int>(scroll_buttons.size()), SCROLL_ITEM_EXTENT,
				SCROLL_ITEM_SPACING);
			item_position.z = 3.0f;
			item.set_position(item_position);
			const bool visible = item_scroll.is_item_visible(
				item_position, SCROLL_ITEM_EXTENT, 130.0f);
			item.set_visible(visible);
			if (visible)
				item.update(mouse_inside_scroll ? input : nullptr);
		}

		const float wheel = input->get_mouse_wheel_y();
		if (wheel != 0.0f && !scroll_used_wheel)
			zoom += wheel * 0.05f;
		camera->zoom += (zoom - camera->zoom) * 0.1f;
		camera->position += (camera_target_position - camera->position) / 5;
	}

	void render() {
		this->sprite_shader->bind();

		this->engine->set_framebuffer(game_framebuffer);
		renderer->clear(Color::white());

		Vector mouse_world_pos = screen_to_world(this->camera, game_framebuffer,
			Vector(input->get_mouse_x(), input->get_mouse_y()));
		mouse_world_pos.z = 3;

		const int width = 200, height = 200;

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				renderer->render_image(texture,
					Vector((-width * 0.5f + j) * MS,
						(-height * 0.5f + i) * MS, 0),
					MS, MS, rotation);
			}
		}

		renderer->set_color(Color::white());
		renderer->render_rect(Vector(this->camera->get_resolution_x() * 0.5f, 0, 0),
			MS, MS, rotation);

		test_obj->render();

		flush_render_context();
		renderer->apply_pixel_perfect_camera(game_framebuffer);

		this->engine->set_framebuffer(ui_framebuffer);
		renderer->clear(0.0f, 0.0f, 0.0f, 0.0f);
		this->sprite_shader->bind();
		renderer->set_color(Color::white());
		renderer->render_ui_font(font, "this is for test.", Vector(camera->get_resolution_x() * -0.5f, 300, 1), 0.0f, "left", Color::black(), 4.0f);
		renderer->render_ui_font(font, "fps : " + std::to_string(this->engine->get_frames()), Vector(camera->get_resolution_x() * -0.5f, 250, 1), 0.0f, "left", Color::black(), 4.0f);
		renderer->render_ui_font(font,
			"sfx voices : " +
			std::to_string(click->get_playing_instance_count()),
			Vector(camera->get_resolution_x() * -0.5f, 200, 1), 0.0f,
			"left", Color::black(), 4.0f);
		renderer->render_ui_font(font, "C : play 4 overlapping SFX",
			Vector(camera->get_resolution_x() * -0.5f, 150, 1), 0.0f,
			"left", Color::black(), 4.0f);

		const Color main_panel_color = ui_button->is_pressed() ?
			Color(0.55f, 0.25f, 0.25f, 0.95f) :
			(ui_button->is_hovered() ?
				Color(0.42f, 0.22f, 0.22f, 0.95f) :
				Color(0.18f, 0.18f, 0.24f, 0.92f));
		renderer->set_color(main_panel_color);
		renderer->render_ui_rect(Vector(0.0f, 80.0f, 2.0f),
			140.0f, 170.0f);
		ui_button->render(renderer);
		renderer->set_color(Color::white());
		renderer->render_ui_font(font, "BUTTON",
			Vector(0.0f, -25.0f, 4.0f));

		renderer->set_color(Color(0.08f, 0.09f, 0.13f, 0.94f));
		const Vector scroll_position = item_scroll.get_position();
		renderer->render_ui_rect(Vector(scroll_position.x, scroll_position.y,
			1.0f), 820.0f, 170.0f);
		renderer->push_ui_mask(scroll_position, item_scroll.get_width(),
			item_scroll.get_height());
		for (int i = 0; i < static_cast<int>(scroll_buttons.size()); ++i) {
			UIButton &item = scroll_buttons[i];
			if (!item.is_visible())
				continue;

			Color item_color = i == selected_item ?
				Color(0.72f, 0.48f, 0.16f, 0.96f) :
				Color(0.18f, 0.22f, 0.30f, 0.96f);
			if (item.is_hovered())
				item_color = Color(0.34f, 0.40f, 0.52f, 0.98f);
			renderer->set_color(item_color);
			const Vector &position = item.get_position();
			renderer->render_ui_rect(Vector(position.x, position.y, 2.5f),
				SCROLL_ITEM_EXTENT, 130.0f);
			item.render(renderer);
			renderer->set_color(Color::white());
			renderer->render_ui_font(font, std::to_string(i + 1),
				Vector(position.x, position.y - 48.0f, 4.0f));
		}
		renderer->pop_ui_mask();

		const int scroll_percent = static_cast<int>(std::round(
			item_scroll.get_normalized_offset() * 100.0f));
		renderer->set_color(Color::white());
		renderer->render_ui_font(font,
			"Hover + click / wheel over list (" +
			std::to_string(scroll_percent) + "%)",
			Vector(0.0f, -345.0f, 4.0f));

		renderer->set_color(Color(1, 0, 0, 1));
		renderer->render_rect(mouse_world_pos, MS, MS, rotation);

		flush_render_context();

		this->engine->set_framebuffer(nullptr);
		FrameBuffer::bind_screen_framebuffer();
		this->blend_shader->bind();
		this->blend_shader->upload_int("uWorldTex", 0);
		this->blend_shader->upload_int("uUITex", 1);
		this->game_framebuffer->bind_texture(0);
		this->ui_framebuffer->bind_texture(1);
		ScreenQuad::render_current_viewport();

		flush_render_context();
	}
};

int main() {
	int width = 1280, height = 720;

	Engine *engine = new Engine();
	engine->init_engine("title", width, height);

	engine->set_scene(new Game(engine));

	engine->start();

	return 0;
}
*/