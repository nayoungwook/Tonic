#include "engine/engine.h"

#include "engine/camera.h"
#include "engine/display.h"
#include "engine/framebuffer.h"
#include "engine/input.h"
#include "engine/renderer.h"
#include "engine/render_context.h"
#include "engine/scene.h"
#include "engine/shader.h"
#include "engine/shader_manager.h"

#include <algorithm>

void Engine::init_engine(const std::string &title, int width, int height) {

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("SDL_Init Error: %s\n", SDL_GetError());
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
		SDL_GL_CONTEXT_PROFILE_CORE);

	this->width = width;
	this->height = height;

	Camera *camera = new Camera((float)width, (float)height);
	Display *display = new Display(title, width, height);
	display->set_camera(camera);
	display->update_viewport(width, height);
	Input *input = new Input();

	default_frame_buffer = new FrameBuffer();

	ShaderManager *shader_manager = new ShaderManager();

	Shader *default_unlit_shader =
		new Shader("shaders/sprite.vert", "shaders/sprite.frag");
	Shader *plain_shape_shader =
		new Shader("shaders/default.vert", "shaders/default.frag");

	shader_manager->fetch_shader("default_unlit", default_unlit_shader);
	shader_manager->fetch_shader("default", plain_shape_shader);

	Renderer *renderer = new Renderer(
		this, camera, shader_manager->get_shader("default_unlit"));

	this->current_frame_buffer = default_frame_buffer;
	this->display = display;
	this->camera = camera;
	this->renderer = renderer;
	this->shader_manager = shader_manager;
	this->input = input;
}

float Engine::get_width() { return width; }

float Engine::get_height() { return height; }

Input *Engine::get_input() { return input; }

Scene *Engine::get_current_scene() { return current_scene; }

void Engine::set_scene(Scene *scene) {
	this->current_scene = scene;
	scene->init();
}

FrameBuffer *Engine::get_frame_buffer() { return current_frame_buffer; }

void Engine::set_frame_buffer(FrameBuffer *frame_buffer) {
	if (frame_buffer == nullptr) {
		this->current_frame_buffer = default_frame_buffer;
		return;
	}
	this->current_frame_buffer = frame_buffer;
}

void Engine::start() {
	bool running = true;
	Uint64 previous = SDL_GetPerformanceCounter();
	double lag = 0.0;

	const int TARGET_FPS = 60;
	const float TIME_STEP = 1.0f / TARGET_FPS;

	SDL_Event event;
	int frames = 0;
	int updates = 0;
	double fps_timer = 0.0;

	while (running) {
		renderer->begin_frame();
		if (current_scene != nullptr) {
			current_scene->start_frame();
		}
		input->begin_new_frame();

		while (SDL_PollEvent(&event)) {

			input->process_event(event, this);

			switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;

			case SDL_WINDOWEVENT:
				if (event.window.event ==
					SDL_WINDOWEVENT_SIZE_CHANGED) {
					int w = event.window.data1;
					int h = event.window.data2;
					this->display->update_viewport(w, h);
				}
			}
		}

		Uint64 current = SDL_GetPerformanceCounter();
		double elapsed = (double)(current - previous) /
			SDL_GetPerformanceFrequency();
		previous = current;
		lag += elapsed;
		fps_timer += elapsed;

		while (lag >= TIME_STEP) {
			if (current_scene != nullptr) {
				current_scene->update();
			}
			updates++;
			lag -= TIME_STEP;
		}

		if (current_scene != nullptr) {
			bool pixel_perfect_screen =
				display->is_pixel_perfect_screen() &&
				camera->is_pixel_perfect_enabled();

			FrameBuffer *previous_frame_buffer = current_frame_buffer;

			if (pixel_perfect_screen) {
				// if pixel perfect screen enabled, we have to adjust screen resolution lower.
				int buffer_width =
					camera->get_pixel_buffer_width();
				int buffer_height =
					camera->get_pixel_buffer_height();

				default_frame_buffer->set_pixel_perfect(true);
				default_frame_buffer->resize(buffer_width, buffer_height);
				set_frame_buffer(default_frame_buffer);
				camera->begin_pixel_perfect_render();
				default_frame_buffer->bind();
			}
			else {
				// Otherwise, restore the framebuffer to the camera resolution.
				default_frame_buffer->set_pixel_perfect(false);
				FrameBuffer::resize_camera_sized_framebuffers();
				set_frame_buffer(default_frame_buffer);
				default_frame_buffer->bind();
			}

			// render and flush.
			current_scene->render();
			current_scene->flush_render_context();

			if (pixel_perfect_screen) {
				camera->end_pixel_perfect_render();
			}

			// Ready to render in fbo 0
			FrameBuffer::bind_screen_framebuffer();
			display->apply_screen_viewport();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Ready to render default_frame_buffer
			RenderContext present_context =
				gen_framebuffer_render_context(nullptr,
					default_frame_buffer,
					Vector(0, 0),
					0.0f,
					camera->get_width(),
					camera->get_height(),
					glm::vec4(1.0f),
					true);

			if (pixel_perfect_screen) {
				// to prevent jittering, we have to offset uv little bit.
				present_context.uv = camera->get_pixel_source_uv();
			}

			// flush
			current_scene->add_render_context(present_context);
			current_scene->flush_render_context();
			current_frame_buffer = previous_frame_buffer;
		}

		frames++;
		SDL_GL_SwapWindow(display->get_window());

		if (fps_timer >= 1.0) {
			printf("FPS: %d | UPS: %d\n", frames, updates);
			frames = 0;
			updates = 0;
			fps_timer -= 1.0;
		}
	}
}

ShaderManager *Engine::get_shader_manager() { return this->shader_manager; }

Renderer *Engine::get_renderer() { return this->renderer; }

Camera *Engine::get_camera() { return this->camera; }

Display *Engine::get_display() {
	return this->display;
	;
}
