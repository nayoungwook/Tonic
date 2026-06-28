#pragma once

class FrameBuffer;

class ScreenQuad {
public:
	static void init();
	static void render(FrameBuffer *frame_buffer = nullptr);
	static void render_current_viewport();
	static void dispose();

private:
	static unsigned vao;
	static unsigned vbo;
	static unsigned ebo;
};
