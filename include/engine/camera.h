#pragma once

#include <GL/glew.h>
#include "engine/mathf.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
	Camera(int width, int height);

	Vector position;
	float zoom = 1.0f;
	float rotation = 0.0f;

	const glm::mat4 &get_view_projection();
	const glm::mat4 &get_view() const;
	const glm::mat4 &get_projection() const;
	glm::mat4 get_pixel_view_projection(float reference_width,
		float reference_height, float assets_pixels_per_unit) const;
	const int get_width() const;
	const int get_height() const;
	const int get_base_width() const;
	const int get_base_height() const;
	void calculate_matrix();
	void configure_pixel_perfect(float reference_width, float reference_height,
		float assets_pixels_per_unit);
	void configure_pixel_perfect_for_sprite(int sprite_pixel_size,
		float world_size);
	void set_pixel_perfect_enabled(bool enabled);
	bool is_pixel_perfect_enabled() const;
	void set_assets_pixels_per_unit(float value);
	float get_assets_pixels_per_unit() const;
	float get_world_units_per_pixel() const;
	int get_pixel_view_width() const;
	int get_pixel_view_height() const;
	int get_pixel_buffer_width() const;
	int get_pixel_buffer_height() const;
	glm::vec4 get_pixel_source_uv() const;
	void begin_pixel_perfect_render();
	void end_pixel_perfect_render();

	bool is_dirty();

private:
	glm::mat4 build_projection(float projection_width,
		float projection_height) const;
	Vector get_pixel_snapped_center(float projection_width,
		float projection_height) const;

	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 view_projection;

	Vector cached_position;
	float cached_zoom = 1.0f;
	float cached_rotation = 0.0f;
	bool cached_pixel_rendering = false;
	float cached_assets_pixels_per_unit = 1.0f;

	int width, height;
	int base_width, base_height;
	float assets_pixels_per_unit = 1.0f;
	bool pixel_perfect_enabled = false;
	bool pixel_rendering = false;
};
