#include "engine/texture.h"
#include "engine/camera.h"
#include "engine/error.h"
#include "engine/renderer.h"
#include "engine/shader.h"

#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Texture::Texture(const std::string &path) : path(path) {
        this->init_texture();
        this->load_texture();
}

void Texture::fetch_atlas(glm::vec4 atlas) {
        this->atlas_data.push_back(atlas);
}

void Texture::set_atlas(int slot) {
        if (this->atlas_data.size() == 0)
                return;

        if (this->atlas_data.size() <= slot || slot < 0) {
                error("Atlas out of index MAX : " +
                      std::to_string(this->atlas_data.size()) + " , " +
                      std::to_string(slot));
        }

        glm::vec4 atlas = this->atlas_data[slot];

        float x = atlas.x / width;
        float y = atlas.y / height;
        float w = atlas.z / width;
        float h = atlas.w / height;

        float vertices[] = {// pos       // uv
                            -0.5f, 0.5f,  x,    y,     0.5f,  0.5f,
                            x + w, y,     0.5f, -0.5f, x + w, y + h,
                            -0.5f, -0.5f, x,    y + h};

        unsigned int indices[] = {0, 1, 2, 2, 3, 0};

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                     GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                     GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                              (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                              (void *)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
}

void Texture::init_texture() {
        float vertices[] = {// pos       // uv
                            -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  1.0f, 0.0f,
                            0.5f,  -0.5f, 1.0f, 1.0f, -0.5f, -0.5f, 0.0f, 1.0f};

        unsigned int indices[] = {0, 1, 2, 2, 3, 0};

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                     GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                     GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                              (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                              (void *)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
}

void Texture::load_texture() {
        std::string full_path = std::filesystem::current_path().string();
        full_path += "\\" + path;

        unsigned char *data =
            stbi_load(full_path.c_str(), &width, &height, &channels, 0);

        if (!data) {
                error("Failed to load texture " + full_path);
                return;
        }

        glGenTextures(1, &this->tex);
        glBindTexture(GL_TEXTURE_2D, this->tex);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                     GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
}

void Texture::bind(unsigned int slot) {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, this->tex);
}

void Texture::bind() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->tex);
}

const unsigned int Texture::get_tex() const { return tex; }

void Texture::render(Renderer *renderer, glm::mat4 &model) {
        Shader *shader = renderer->get_shader();
        Camera *camera = renderer->get_camera();

        assert(shader != nullptr);
        assert(camera != nullptr);

        shader->bind();
        this->bind(0);

        glUniformMatrix4fv(shader->get_uniform_location("uModel"), 1, GL_FALSE,
                           glm::value_ptr(model));

        glBindVertexArray(this->vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}