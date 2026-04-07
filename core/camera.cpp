#include "engine/camera.h"

Camera::Camera(int width, int height) : width(width), height(height) {

        this->position = glm::vec2(0, 0);
        this->rotation = 0;
        this->zoom = 1.0f;

        this->projection =
            glm::ortho(-width * 0.5f, width * 0.5f, -height * 0.5f,
                       height * 0.5f, -1.0f, 1.0f);

        glViewport(0, 0, width, height);
        calculate_matrix();
}

bool Camera::is_dirty() {
        if (this->cached_position.x != this->position.x ||
            this->cached_position.y != this->position.y)
                return true;
        return false;
}

void Camera::calculate_matrix() {
        glm::mat4 transform = glm::mat4(1.0f);

        transform = glm::translate(transform, glm::vec3(-position, 0.0f));
        transform =
            glm::rotate(transform, glm::radians(rotation), glm::vec3(0, 0, 1));
        transform =
            glm::scale(transform, glm::vec3(1.0f / zoom, 1.0f / zoom, 1.0f));

        this->view = transform;
        this->view_projection = this->projection * this->view;
}

const int Camera::get_width() const { return width; }

const int Camera::get_height() const { return height; }

const glm::mat4 &Camera::get_view_projection() { return view_projection; }