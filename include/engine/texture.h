#pragma once

#include <GL/glew.h>
#include <SDL/SDL.h>

#include <filesystem>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>

class Renderer;

class Texture {
      public:
        void render(Renderer *renderer, glm::mat4 &model);
        Texture(const std::string &path);

        const unsigned int get_tex() const;

        void bind();
        void bind(unsigned int slot);

        void fetch_atlas(glm::vec4 atlas);
        void set_atlas(int slot);

      private:
        std::vector<glm::vec4> atlas_data;
        int atlas_slot;

        unsigned int tex;
        void load_texture();
        int width, height, channels;
        std::string path;

        unsigned vao, vbo, ebo;
        void init_texture();
};