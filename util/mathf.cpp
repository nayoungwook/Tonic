#include "engine/mathf.h"
#include "engine/camera.h"

#include <algorithm>
#include <cmath>

Vector::Vector(float x, float y, float z) : x(x), y(y), z(z) {};
Vector::Vector(float x, float y) : x(x), y(y), z(0) {};
Vector::Vector() : x(0), y(0), z(0) {};

Vector Vector::clone_vector() const { return Vector(x, y, z); }

Vector Vector::normalized() const {
        float length = (float)sqrt(x * x + y * y + z * z);
        if (length == 0)
                return Vector(0, 0, 0);
        return Vector(x / length, y / length, z / length);
}

void Vector::set_transform(float x, float y) {
        this->x = x;
        this->y = y;
}
void Vector::set_transform(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
}
void Vector::translate(float xv, float yv) {
        x += xv;
        y += yv;
}
void Vector::translate(float xv, float yv, float zv) {
        x += xv;
        y += yv;
        z += zv;
}

Vector Vector::operator+(const float other) {
        return Vector(x + other, y + other, z + other);
}

Vector Vector::operator-(const float other) {
        return Vector(x - other, y - other, z - other);
}

Vector Vector::operator/(const float other) {
        return Vector(x / other, y / other, z / other);
}

Vector Vector::operator*(const float other) {
        return Vector(x * other, y * other, z * other);
}

Vector Vector::operator%(const int other) {
        return Vector((int)x % other, (int)y % other, (int)z % other);
}

Vector Vector::operator+(const Vector other) {
        return Vector(x + other.x, y + other.y, z + other.z);
}

Vector Vector::operator-(const Vector other) {
        return Vector(x - other.x, y - other.y, z - other.z);
}

Vector Vector::operator/(const Vector other) {
        return Vector(x / other.x, y / other.y, z / other.z);
}

Vector Vector::operator*(const Vector other) {
        return Vector(x * other.x, y * other.y, z * other.z);
}

Vector Vector::operator%(const Vector other) {
        return Vector((int)x % (int)other.x, (int)y % (int)other.y,
                      (int)z % (int)other.z);
}

Vector &Vector::operator+=(const Vector &other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
}

Vector &Vector::operator-=(const Vector &other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
}

Vector Vector::normalize() {
        return normalized();
}

float Vector::dist2d(const Vector other) const {
        return sqrt((x - other.x) * (x - other.x) +
                    (y - other.y) * (y - other.y));
}

float Vector::dist(const Vector other) const {
        return sqrt((x - other.x) * (x - other.x) +
                    (y - other.y) * (y - other.y) +
                    (z - other.z) * (z - other.z));
}

Vector world_to_screen(Camera *camera, const Vector &world_pos) {
	if (camera->is_pixel_perfect_transform_enabled()) {
		float units_per_pixel =
			std::max(0.0001f, camera->get_pixel_perfect_units_per_pixel());
		float render_width =
			std::max(1.0f, camera->get_pixel_perfect_render_width());
		float render_height =
			std::max(1.0f, camera->get_pixel_perfect_render_height());
		float camera_width = std::max(1.0f,
			static_cast<float>(camera->get_width()));
		float camera_height = std::max(1.0f,
			static_cast<float>(camera->get_height()));

		float source_x = (world_pos.x - camera->position.x) /
			units_per_pixel;
		float source_y = (world_pos.y - camera->position.y) /
			units_per_pixel;
		float cosine = std::cos(camera->rotation);
		float sine = std::sin(camera->rotation);
		float zoom = camera->zoom;

		float output_x = zoom * (cosine * source_x + sine * source_y);
		float output_y = zoom * (-sine * source_x + cosine * source_y);

		return Vector(output_x * camera_width / render_width,
			output_y * camera_height / render_height, world_pos.z);
	}

        glm::vec4 clip = camera->get_view_projection() *
                         glm::vec4(world_pos.x, world_pos.y, 0.0f, 1.0f);

        glm::vec3 ndc = glm::vec3(clip) / clip.w;

        Vector screen;

        screen.x = ndc.x * (camera->get_width() * 0.5f);
        screen.y = ndc.y * (camera->get_height() * 0.5f);

        screen.z = world_pos.z;
        return screen;
}

Vector screen_to_world(Camera *camera, const Vector &screen_pos) {
	if (camera->is_pixel_perfect_transform_enabled()) {
		float units_per_pixel =
			std::max(0.0001f, camera->get_pixel_perfect_units_per_pixel());
		float render_width =
			std::max(1.0f, camera->get_pixel_perfect_render_width());
		float render_height =
			std::max(1.0f, camera->get_pixel_perfect_render_height());
		float camera_width = std::max(1.0f,
			static_cast<float>(camera->get_width()));
		float camera_height = std::max(1.0f,
			static_cast<float>(camera->get_height()));
		float output_x = screen_pos.x * render_width / camera_width;
		float output_y = screen_pos.y * render_height / camera_height;
		float zoom = camera->zoom;

		if (std::abs(zoom) < 0.0001f)
			zoom = zoom < 0.0f ? -0.0001f : 0.0001f;

		float cosine = std::cos(camera->rotation);
		float sine = std::sin(camera->rotation);
		float source_x = (cosine * output_x - sine * output_y) / zoom;
		float source_y = (sine * output_x + cosine * output_y) / zoom;

		return Vector(camera->position.x + source_x * units_per_pixel,
			camera->position.y + source_y * units_per_pixel,
			screen_pos.z);
	}

        glm::vec2 ndc;

        ndc.x = screen_pos.x / (camera->get_width() * 0.5f);
        ndc.y = screen_pos.y / (camera->get_height() * 0.5f);

        glm::vec4 clip = glm::vec4(ndc, 0.0f, 1.0f);

        glm::mat4 inv = glm::inverse(camera->get_view_projection());
        glm::vec4 world = inv * clip;
        glm::vec2 res = glm::vec2(world) / world.w;

        return Vector(res.x, res.y, screen_pos.z);
}

float get_distance(const Vector &position, const Vector &position2) {
        return position.dist(position2);
}

float get_distance_squared(const Vector &position, const Vector &position2) {
        float x = position.x - position2.x;
        float y = position.y - position2.y;
        float z = position.z - position2.z;
        return x * x + y * y + z * z;
}

float get_angle(const Vector &position, const Vector &position2) {
        return std::atan2(position2.y - position.y, position2.x - position.x);
}

float get_xv(float move_speed, const Vector &position,
             const Vector &position2) {
        return std::cos(get_angle(position, position2)) * move_speed;
}

float get_yv(float move_speed, const Vector &position,
             const Vector &position2) {
        return std::sin(get_angle(position, position2)) * move_speed;
}

float dot(const Vector &v1, const Vector &v2) {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
