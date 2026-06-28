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
		cached_rotation != rotation ||
		cached_pixel_rendering != pixel_rendering ||
		cached_assets_pixels_per_unit != assets_pixels_per_unit;
}

void Camera::calculate_matrix() {
	if (!is_dirty())
		return;

	float projection_width = static_cast<float>(width);
	float projection_height = static_cast<float>(height);
	Vector render_position = position;

	if (pixel_rendering && pixel_perfect_enabled) {
		projection_width =
			get_pixel_buffer_width() * get_world_units_per_pixel();
		projection_height =
			get_pixel_buffer_height() * get_world_units_per_pixel();

		render_position =
			get_pixel_snapped_center(projection_width, projection_height);
	}

	projection = build_projection(projection_width, projection_height);

	view = glm::mat4(1.0f);
	view = glm::scale(view,
		glm::vec3(zoom, zoom, 1.0f));
	view = glm::rotate(view,
		-rotation, glm::vec3(0.0f, 0.0f, 1.0f));
	view = glm::translate(view,
		glm::vec3(-render_position.x,
			-render_position.y,
			0.0f));

	view_projection = projection * view;

	cached_position = position;
	cached_zoom = zoom;
	cached_rotation = rotation;
	cached_pixel_rendering = pixel_rendering;
	cached_assets_pixels_per_unit = assets_pixels_per_unit;
}
const int Camera::get_width() const { return width; }

const int Camera::get_height() const { return height; }

const int Camera::get_base_width() const { return base_width; }

const int Camera::get_base_height() const { return base_height; }

const glm::mat4 &Camera::get_view_projection() { return view_projection; }

const glm::mat4 &Camera::get_projection() const { return projection; }

void Camera::configure_pixel_perfect(float reference_width,
	float reference_height, float assets_pixels_per_unit) {
	if (reference_width <= 0.0f || reference_height <= 0.0f)
		return;

	width = std::max(1, static_cast<int>(std::round(
		reference_width / std::max(0.0001f, assets_pixels_per_unit))));
	height = std::max(1, static_cast<int>(std::round(
		reference_height / std::max(0.0001f, assets_pixels_per_unit))));
	set_assets_pixels_per_unit(assets_pixels_per_unit);
	set_pixel_perfect_enabled(true);
	cached_zoom = -1.0f;
}

void Camera::configure_pixel_perfect_for_sprite(int sprite_pixel_size,
	float world_size) {
	if (sprite_pixel_size <= 0 || world_size <= 0.0f)
		return;

	set_assets_pixels_per_unit(sprite_pixel_size / world_size);
	set_pixel_perfect_enabled(true);
}

void Camera::set_pixel_perfect_enabled(bool enabled) {
	if (pixel_perfect_enabled == enabled)
		return;
	pixel_perfect_enabled = enabled;
	cached_zoom = -1.0f;
}

bool Camera::is_pixel_perfect_enabled() const {
	return pixel_perfect_enabled;
}

void Camera::set_assets_pixels_per_unit(float value) {
	assets_pixels_per_unit = std::max(0.0001f, value);
	cached_zoom = -1.0f;
}

float Camera::get_assets_pixels_per_unit() const {
	return assets_pixels_per_unit;
}

float Camera::get_world_units_per_pixel() const {
	return 1.0f / assets_pixels_per_unit;
}

int Camera::get_pixel_view_width() const {
	return std::max(1, static_cast<int>(std::ceil(
		static_cast<float>(width) * assets_pixels_per_unit)));
}

int Camera::get_pixel_view_height() const {
	return std::max(1, static_cast<int>(std::ceil(
		static_cast<float>(height) * assets_pixels_per_unit)));
}

int Camera::get_pixel_buffer_width() const {
	return get_pixel_view_width() + 1;
}

int Camera::get_pixel_buffer_height() const {
	return get_pixel_view_height() + 1;
}

Vector Camera::get_pixel_snapped_center(float projection_width,
	float projection_height) const {
	float units_per_pixel = get_world_units_per_pixel();
	float left = position.x - width * 0.5f;
	float bottom = position.y - height * 0.5f;
	float snapped_left = std::floor(left / units_per_pixel) *
		units_per_pixel;
	float snapped_bottom = std::floor(bottom / units_per_pixel) *
		units_per_pixel;

	return Vector(snapped_left + projection_width * 0.5f,
		snapped_bottom + projection_height * 0.5f, position.z);
}

glm::vec4 Camera::get_pixel_source_uv() const {
	float units_per_pixel = get_world_units_per_pixel();
	float left = position.x - width * 0.5f;
	float bottom = position.y - height * 0.5f;
	float snapped_left = std::floor(left / units_per_pixel) *
		units_per_pixel;
	float snapped_bottom = std::floor(bottom / units_per_pixel) *
		units_per_pixel;
	float frac_x = (left - snapped_left) / units_per_pixel;
	float frac_y = (bottom - snapped_bottom) / units_per_pixel;

	float buffer_width = static_cast<float>(get_pixel_buffer_width());
	float buffer_height = static_cast<float>(get_pixel_buffer_height());
	float view_width = static_cast<float>(get_pixel_view_width());
	float view_height = static_cast<float>(get_pixel_view_height());

	return glm::vec4(frac_x / buffer_width, frac_y / buffer_height,
		(frac_x + view_width) / buffer_width,
		(frac_y + view_height) / buffer_height);
}

void Camera::begin_pixel_perfect_render() {
	pixel_rendering = true;
	cached_zoom = -1.0f;
}

void Camera::end_pixel_perfect_render() {
	pixel_rendering = false;
	cached_zoom = -1.0f;
	calculate_matrix();
}
