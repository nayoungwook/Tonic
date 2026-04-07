#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
      public:
        Camera(int width, int height);

        glm::vec2 position;
        float zoom = 1.0f;
        float rotation = 0.0f;

        const glm::mat4 &get_view_projection();
        const int get_width() const;
        const int get_height() const;
        void calculate_matrix();

        bool is_dirty();

      private:
        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 view_projection;

        glm::vec2 cached_position;

        int width, height;
};