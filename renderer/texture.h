#pragma once

#include "color.h"

#include <GL/glew.h>
#include <SDL3/SDL.h>

#include <iostream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>

class Renderer;

struct TextureInstance {
	glm::vec4 transform;
	Color color;
	glm::vec4 uv;
	float rotation;
};

class Texture {
public:
	void render(Renderer *renderer, const glm::mat4 &model);
	void render_instanced(Renderer *renderer,
		const TextureInstance *instances, int count);
	Texture(const std::string &path);
	Texture(int width, int height, const unsigned char *pixels);
	Texture(unsigned int texture_id, int width, int height, bool owns_texture);
	~Texture();

	const unsigned int get_tex() const;

	int get_atlas_slot();

	void bind();
	void bind(unsigned int slot);

	void fetch_atlas(glm::vec4 atlas);
	void set_atlas(int slot);
	glm::vec4 get_atlas_uv(int slot) const;
	void set_filter(GLenum min_filter, GLenum mag_filter);
	int get_width() const;
	int get_height() const;

private:
	struct BatchVertex {
		float x, y, z;
		float u, v;
		unsigned char r, g, b, a;
	};

	std::vector<glm::vec4> atlas_data;
	int atlas_slot = -1;
	bool owns_texture = true;

	unsigned int tex = 0;
	void load_texture();
	void upload_pixels(const unsigned char *pixels, GLenum format);
	int width, height, channels;
	std::string path;

	unsigned vao = 0, vbo = 0, ebo = 0;
	void init_texture();
	std::vector<BatchVertex> batch_vertices;
	std::vector<unsigned> batch_indices;
};
