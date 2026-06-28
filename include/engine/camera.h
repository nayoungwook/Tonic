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
	const glm::mat4 &get_pixel_perfect_view_projection();
	glm::mat4 get_pixel_perfect_view_projection(float units_per_pixel);
	glm::mat4 get_pixel_perfect_view_projection(float units_per_pixel,
		float buffer_width, float buffer_height);
	const glm::mat4 &get_view() const;
	const glm::mat4 &get_projection() const;
	glm::mat4 get_screen_projection() const;

	const glm::vec2 &get_jitter_offset() const;
	glm::vec2 get_jitter_offset(float units_per_pixel) const;

	const int get_width() const;
	const int get_height() const;
	const int get_resolution_x() const;
	const int get_resolution_y() const;
	const int get_base_width() const;
	const int get_base_height() const;
	bool is_pixel_perfect_transform_enabled() const;
	float get_pixel_perfect_units_per_pixel() const;
	float get_pixel_perfect_render_width() const;
	float get_pixel_perfect_render_height() const;

	void calculate_matrix();
	void configure_pixel_perfect_transform(bool enabled,
		float units_per_pixel = 1.0f, float render_width = 0.0f,
		float render_height = 0.0f);

	bool is_dirty();

private:
	glm::mat4 build_projection(float projection_width,
		float projection_height) const;

	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 view_projection;
	glm::mat4 pixel_perfect_view_projection;

	glm::vec2 jitter_offset;

	Vector cached_position;
	float cached_zoom = 1.0f;
	float cached_rotation = 0.0f;

	int width, height;
	int base_width, base_height;

	bool pixel_perfect_transform_enabled = false;
	float pixel_perfect_units_per_pixel = 1.0f;
	float pixel_perfect_render_width = 0.0f;
	float pixel_perfect_render_height = 0.0f;
};
