#include "engine/camera.h"

#include <algorithm>
#include <cmath>

Camera::Camera(int width, int height)
	: width(width), height(height), base_width(width), base_height(height) {
	position = Vector(0, 0);
	rotation = 0.0f;
	zoom = 1.0f;
	cached_zoom = -1.0f;

	projection = build_projection(static_cast<float>(width),
		static_cast<float>(height));

	glViewport(0, 0, width, height);
	calculate_matrix();
}

glm::mat4 Camera::build_projection(float projection_width,
	float projection_height) const {
	return glm::ortho(-projection_width * 0.5f,
		projection_width * 0.5f, -projection_height * 0.5f,
		projection_height * 0.5f, -1.0f, 1.0f);
}

bool Camera::is_dirty() {
	return cached_position.x != position.x ||
		cached_position.y != position.y ||
		cached_zoom != zoom ||
		cached_rotation != rotation;
}

const glm::vec2 &Camera::get_jitter_offset() const {
	return jitter_offset;
}

glm::vec2 Camera::get_jitter_offset(float units_per_pixel) const {
	units_per_pixel = std::max(0.0001f, units_per_pixel);
	float snapped_x = std::floor(position.x / units_per_pixel) *
		units_per_pixel;
	float snapped_y = std::floor(position.y / units_per_pixel) *
		units_per_pixel;

	return glm::vec2((position.x - snapped_x) / units_per_pixel,
		(position.y - snapped_y) / units_per_pixel);
}

void Camera::calculate_matrix() {
	if (!is_dirty())
		return;

	float safe_zoom = std::max(0.01f, zoom);
	float projection_width = static_cast<float>(width) / safe_zoom;
	float projection_height = static_cast<float>(height) / safe_zoom;

	projection = build_projection(projection_width, projection_height);

	view = glm::mat4(1.0f);

	view = glm::rotate(view,
		-rotation, glm::vec3(0.0f, 0.0f, 1.0f));
	view = glm::translate(view,
		glm::vec3(-position.x,
			-position.y,
			0.0f));
	view_projection = projection * view;

	float snapped_x = std::floor(position.x);
	float snapped_y = std::floor(position.y);

	glm::mat4 pixel_perfect_view = glm::mat4(1.0f);
	pixel_perfect_view = glm::rotate(pixel_perfect_view,
		-rotation, glm::vec3(0.0f, 0.0f, 1.0f));
	pixel_perfect_view = glm::translate(pixel_perfect_view,
		glm::vec3(-snapped_x, -snapped_y, 0.0f));

	this->jitter_offset = glm::vec2(position.x - snapped_x,
		position.y - snapped_y);
	pixel_perfect_view_projection = projection * pixel_perfect_view;

	cached_position = position;
	cached_zoom = zoom;
	cached_rotation = rotation;
}


const int Camera::get_width() const { return width; }

const int Camera::get_height() const { return height; }

const int Camera::get_resolution_x() const { return width; }

const int Camera::get_resolution_y() const { return height; }

const int Camera::get_base_width() const { return base_width; }

const int Camera::get_base_height() const { return base_height; }

const glm::mat4 &Camera::get_view_projection() { return view_projection; }
const glm::mat4 &Camera::get_pixel_perfect_view_projection() { return pixel_perfect_view_projection; }

glm::mat4 Camera::get_pixel_perfect_view_projection(float units_per_pixel) {
	if (is_dirty())
		calculate_matrix();

	units_per_pixel = std::max(0.0001f, units_per_pixel);
	float snapped_x = std::floor(position.x / units_per_pixel) *
		units_per_pixel;
	float snapped_y = std::floor(position.y / units_per_pixel) *
		units_per_pixel;

	glm::mat4 pixel_perfect_view = glm::mat4(1.0f);
	pixel_perfect_view = glm::rotate(pixel_perfect_view,
		-rotation, glm::vec3(0.0f, 0.0f, 1.0f));
	pixel_perfect_view = glm::translate(pixel_perfect_view,
		glm::vec3(-snapped_x, -snapped_y, 0.0f));

	return projection * pixel_perfect_view;
}

glm::mat4 Camera::get_pixel_perfect_view_projection(float units_per_pixel,
	float buffer_width, float buffer_height) {
	if (is_dirty())
		calculate_matrix();

	units_per_pixel = std::max(0.0001f, units_per_pixel);
	buffer_width = std::max(1.0f, buffer_width);
	buffer_height = std::max(1.0f, buffer_height);

	float snapped_x = std::floor(position.x / units_per_pixel) *
		units_per_pixel;
	float snapped_y = std::floor(position.y / units_per_pixel) *
		units_per_pixel;

	glm::mat4 pixel_projection = build_projection(
		buffer_width * units_per_pixel,
		buffer_height * units_per_pixel);
	glm::mat4 pixel_view = glm::mat4(1.0f);
	pixel_view = glm::translate(pixel_view,
		glm::vec3(-snapped_x, -snapped_y, 0.0f));

	return pixel_projection * pixel_view;
}

const glm::mat4 &Camera::get_view() const { return view; }

const glm::mat4 &Camera::get_projection() const { return projection; }

glm::mat4 Camera::get_screen_projection() const {
	return build_projection(static_cast<float>(width),
		static_cast<float>(height));
}
