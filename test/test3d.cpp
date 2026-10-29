
#include <model.h>
#include <engine.h>
#include <framebuffer.h>
#include <algorithm>
#include <vector>

class Game : public Scene {

private:
	static constexpr int RENDER_WIDTH = 320;
	static constexpr int RENDER_HEIGHT = 180;
	float timer = 0.0f;
	Mesh *mesh = nullptr;
	FrameBuffer *framebuffer = nullptr;
	Model *teapot = nullptr;

	void render_world() {
		Camera *camera = engine->get_camera();
		Renderer *renderer = engine->get_renderer();
		Shader *shader = engine->get_shader_manager()->get_shader("model");

		camera->camera_mode = CameraMode::VIEW_3D;
		camera->adjust_projection(RENDER_WIDTH, RENDER_HEIGHT);
		engine->set_framebuffer(framebuffer);
		glEnable(GL_DEPTH_TEST);

		renderer->clear(0.04f, 0.05f, 0.08f, 1.0f);
		renderer->set_color(Color(0.55f, 0.8f, 1.0f));
		renderer->render_mesh(mesh, shader, Vector(0, 0, -20),
			Vector(1, 1, 1), Vector(sin(timer), cos(timer), timer));

		flush_render_context();
	}

	void present_framebuffer() {
		Camera *camera = engine->get_camera();
		Renderer *renderer = engine->get_renderer();
		Display *display = engine->get_display();
		float width = camera->get_resolution_x();
		float height = camera->get_resolution_y();

		camera->camera_mode = CameraMode::VIEW_2D;
		camera->adjust_projection(width, height);
		engine->set_framebuffer(nullptr);

		renderer->clear(Color::black());
		renderer->set_color(Color::white());
		renderer->render_ui_framebuffer(framebuffer, Vector(0, 0), width, height);
		flush_render_context();
	}

public:
	Game(Engine *engine) : Scene(engine) {
		Camera *camera = engine->get_camera();
		camera->camera_mode = CameraMode::VIEW_3D;
		camera->adjust_projection(camera->get_resolution_x(), camera->get_resolution_y());
		framebuffer = new FrameBuffer(RENDER_WIDTH, RENDER_HEIGHT, false);
	}

	~Game() override {
		delete mesh;
		delete framebuffer;
	}

	void init() {
		teapot = new Model("resources/teapot.obj");
		mesh = new Mesh(teapot->get_vertices(), teapot->get_indices(), 3);
	}

	void update() {
		timer += 0.01f;
	}

	void render() {
		Camera *camera = engine->get_camera();
		Renderer *renderer = engine->get_renderer();
		const CameraMode previous_mode = camera->camera_mode;
		const int previous_width = camera->get_resolution_x();
		const int previous_height = camera->get_resolution_y();
		const Color previous_color = renderer->get_color();
		const GLboolean depth_enabled = glIsEnabled(GL_DEPTH_TEST);

		render_world();
		present_framebuffer();

		camera->camera_mode = previous_mode;
		camera->adjust_projection(previous_width, previous_height);
		renderer->set_color(previous_color);
		if (depth_enabled) glEnable(GL_DEPTH_TEST);
	}
};

int main() {
	Engine *engine = new Engine();

	engine->init_engine("3D test", 1280, 720);

	Game *game = new Game(engine);

	engine->set_scene(game);

	engine->start();

	delete game;

	return 0;
}
