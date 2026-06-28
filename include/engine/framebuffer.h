#pragma once

#include <vector>

class Camera;

class FrameBuffer {
public:
	enum class SizeMode {
		RENDER,
		CAMERA,
		FIXED
	};

	FrameBuffer();
	FrameBuffer(bool pixel_perfect);
	explicit FrameBuffer(SizeMode size_mode);
	FrameBuffer(SizeMode size_mode, bool pixel_perfect);
	FrameBuffer(int width, int height);
	FrameBuffer(int width, int height, bool pixel_perfect);
	~FrameBuffer();

	void bind();
	void unbind();
	static void bind_screen_framebuffer();
	static void invalidate_bind_cache();
	void clear(float r = 0.0f, float g = 0.0f, float b = 0.0f,
		float a = 1.0f);
	void resize(int width, int height);

	unsigned get_color_texture() const { return color_texture; }
	unsigned get_fbo() const { return fbo; }
	int get_width() const { return width; }
	int get_height() const { return height; }
	float get_resolution_x() const { return static_cast<float>(width); }
	float get_resolution_y() const { return static_cast<float>(height); }
	int get_pixel_view_width() const { return pixel_view_width; }
	int get_pixel_view_height() const { return pixel_view_height; }

	void create(int width, int height);
	void upload_texture(int tex);
	void bind_texture(int tex);
	void dispose();
	void set_pixel_per_unit(float pixel, float world_size);
	float get_pixel_per_unit();

	bool is_pixel_perfect() const;

	static void resize_camera_sized_framebuffers();
	static void resize_camera_sized_framebuffers(Camera *camera);

private:
	void update_resolution_from_mode(Camera *camera = nullptr);
	void register_dynamic();
	void unregister_dynamic();

	unsigned fbo = 0;
	unsigned color_texture = 0;
	unsigned rbo = 0;

	bool pixel_perfect = false;

	float pixel_per_unit = 1.0f;
	int reference_width = 0;
	int reference_height = 0;
	int pixel_view_width = 0;
	int pixel_view_height = 0;

	int width = 0;
	int height = 0;
	bool disposed = false;
	SizeMode size_mode = SizeMode::FIXED;
	bool dynamic_registered = false;
};
