#include "engine/mathf.h"

Vector::Vector(float x, float y, float z) : x(x), y(y), z(z) {};
Vector::Vector(float x, float y) : x(x), y(y), z(0) {};
Vector::Vector() : x(0), y(0), z(0) {};

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

Vector Vector::normalize() {
        float length = (float)sqrt(x * x + y * y + z * z);
        if (length == 0)
                return Vector(0, 0, 0);

        return Vector(x / length, y / length, z / length);
}

float Vector::dist2d(const Vector other) {
        return sqrt((x - other.x) * (x - other.x) +
                    (y - other.y) * (y - other.y));
}

float Vector::dist(const Vector other) {
        return sqrt((x - other.x) * (x - other.x) +
                    (y - other.y) * (y - other.y) +
                    (z - other.z) * (z - other.z));
}

Vector world_to_screen(Camera *camera, const Vector &world_pos) {
        glm::vec4 clip = camera->get_view_projection() *
                         glm::vec4(world_pos.x, world_pos.y, 0.0f, 1.0f);

        glm::vec3 ndc = glm::vec3(clip) / clip.w;

        Vector screen;

        screen.x = ndc.x * (camera->get_width() * 0.5f);
        screen.y = ndc.y * (camera->get_height() * 0.5f);

        return screen;
}

Vector screen_to_world(Camera *camera, const Vector &screen_pos) {
        glm::vec2 ndc;

        ndc.x = screen_pos.x / (camera->get_width() * 0.5f);
        ndc.y = screen_pos.y / (camera->get_height() * 0.5f);

        glm::vec4 clip = glm::vec4(ndc, 0.0f, 1.0f);

        glm::mat4 inv = glm::inverse(camera->get_view_projection());
        glm::vec4 world = inv * clip;
        glm::vec2 res = glm::vec2(world) / world.w;

        return Vector(res.x, res.y, screen_pos.z);
}