#pragma once

#include "camera.h"
#include "glm/glm.hpp"

class Vector {
      private:
      public:
        float x, y, z;

        Vector(float x, float y, float z);
        Vector(float x, float y);
        Vector();

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
        Vector normalize();
        float dist(const Vector other);
        float dist2d(const Vector other);
};

Vector world_to_screen(Camera *camera, const Vector &worldPos);
Vector screen_to_world(Camera *camera, const Vector &screenPos);