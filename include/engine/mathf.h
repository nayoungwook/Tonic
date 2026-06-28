#pragma once

#include "glm/glm.hpp"

class Camera;

class Vector {
      private:
      public:
        float x, y, z;

        Vector(float x, float y, float z);
        Vector(float x, float y);
        Vector();

        Vector clone_vector() const;
        Vector normalized() const;
        void set_transform(float x, float y);
        void set_transform(float x, float y, float z);
        void translate(float xv, float yv);
        void translate(float xv, float yv, float zv);

        Vector operator+(const float other);
        Vector operator-(const float other);
        Vector operator*(const float other);
        Vector operator/(const float other);
        Vector operator%(const int other);
        Vector operator+(const Vector other);
        Vector operator-(const Vector other);
        Vector operator/(const Vector other);
        Vector operator*(const Vector other);
        Vector operator%(const Vector other);
        Vector &operator+=(const Vector &other);
        Vector &operator-=(const Vector &other);
        Vector normalize();
        float dist(const Vector other) const;
        float dist2d(const Vector other) const;
};

Vector world_to_screen(Camera *camera, const Vector &worldPos);
Vector screen_to_world(Camera *camera, const Vector &screenPos);
float get_distance(const Vector &position, const Vector &position2);
float get_distance_squared(const Vector &position, const Vector &position2);
float get_angle(const Vector &position, const Vector &position2);
float get_xv(float move_speed, const Vector &position, const Vector &position2);
float get_yv(float move_speed, const Vector &position, const Vector &position2);
float dot(const Vector &v1, const Vector &v2);
