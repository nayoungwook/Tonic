#include "engine/texture.h"
#include "engine/camera.h"
#include "engine/error.h"
#include "engine/renderer.h"
#include "engine/shader.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace {
std::filesystem::path resolve_resource_path(const std::string &path) {
	std::filesystem::path requested(path);
	if (requested.is_absolute() && std::filesystem::exists(requested))
		return requested;

	std::filesystem::path cwd = std::filesystem::current_path();
	std::array<std::filesystem::path, 4> direct_candidates = {
		cwd / requested,
		cwd / "resources" / requested,
#ifdef ENGINE_RESOURCE_ROOT
		std::filesystem::path(ENGINE_RESOURCE_ROOT) / requested,
#else
		std::filesystem::path(),
#endif
		std::filesystem::path("resources") / requested,
	};

	for (const auto &candidate : direct_candidates) {
		if (!candidate.empty() && std::filesystem::exists(candidate))
			return candidate;
	}

	for (std::filesystem::path probe = cwd; !probe.empty();
		 probe = probe.parent_path()) {
		std::filesystem::path candidate = probe / "resources" / requested;
		if (std::filesystem::exists(candidate))
			return candidate;

		if (probe == probe.parent_path())
			break;
	}

	return cwd / requested;
}
}

Texture::Texture(const std::string &path) : path(path) {
	this->init_texture();
	this->load_texture();
}

Texture::Texture(int width, int height, const unsigned char *pixels)
	: width(width), height(height), channels(4) {
	this->init_texture();
	this->upload_pixels(pixels, GL_RGBA);
}

Texture::Texture(unsigned int texture_id, int width, int height,
	bool owns_texture)
	: width(width), height(height), channels(4), owns_texture(owns_texture),
	  tex(texture_id) {
	this->init_texture();
}

Texture::~Texture() {
	if (instance_vbo != 0)
		glDeleteBuffers(1, &instance_vbo);
	if (ebo != 0)
		glDeleteBuffers(1, &ebo);
	if (vbo != 0)
		glDeleteBuffers(1, &vbo);
	if (vao != 0)
		glDeleteVertexArrays(1, &vao);
	if (tex != 0 && owns_texture)
		glDeleteTextures(1, &tex);
}

void Texture::fetch_atlas(glm::vec4 atlas) {
	this->atlas_data.push_back(atlas);
}

void Texture::set_atlas(int slot) {
	if (this->atlas_data.size() == 0) {
		this->atlas_slot = slot;
		return;
	}

	if (this->atlas_slot == slot)
		return;

	if (this->atlas_data.size() <= slot || slot < 0) {
		error("Atlas out of index MAX : " +
			std::to_string(this->atlas_data.size()) + " , " +
			std::to_string(slot));
	}

	this->atlas_slot = slot;
}

glm::vec4 Texture::get_atlas_uv(int slot) const {
	if (atlas_data.empty() || slot < 0)
		return glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

	if (slot >= static_cast<int>(atlas_data.size()))
		return glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

	glm::vec4 atlas = atlas_data[slot];
	float epsilon = 0.001f;
	float x = (atlas.x + epsilon) / width;
	float y = (atlas.y + epsilon) / height;
	float max_x = (atlas.x + atlas.z - epsilon) / width;
	float max_y = (atlas.y + atlas.w - epsilon) / height;
	return glm::vec4(x, y, max_x, max_y);
}

void Texture::init_texture() {
	float vertices[] = {// pos       // uv
						-0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  1.0f, 0.0f,
						0.5f,  -0.5f, 1.0f, 1.0f, -0.5f, -0.5f, 0.0f, 1.0f };

	unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glGenBuffers(1, &instance_vbo);

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

	glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE,
		sizeof(TextureInstance),
		(void *)offsetof(TextureInstance, transform));
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1);

	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE,
		sizeof(TextureInstance),
		(void *)offsetof(TextureInstance, color));
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);

	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE,
		sizeof(TextureInstance),
		(void *)offsetof(TextureInstance, uv));
	glEnableVertexAttribArray(4);
	glVertexAttribDivisor(4, 1);

	glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE,
		sizeof(TextureInstance),
		(void *)offsetof(TextureInstance, rotation));
	glEnableVertexAttribArray(5);
	glVertexAttribDivisor(5, 1);

	glBindVertexArray(0);
}

void Texture::load_texture() {
	std::filesystem::path full_path = resolve_resource_path(path);

	unsigned char *data =
		stbi_load(full_path.string().c_str(), &width, &height, &channels, 0);

	if (!data) {
		error("Failed to load texture " + full_path.string());
		return;
	}

	GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
	upload_pixels(data, format);

	stbi_image_free(data);
}

void Texture::upload_pixels(const unsigned char *pixels, GLenum format) {
	if (tex == 0)
		glGenTextures(1, &this->tex);
	glBindTexture(GL_TEXTURE_2D, this->tex);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	GLenum internal_format = (format == GL_RGBA) ? GL_RGBA8 : GL_RGB8;
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format,
		GL_UNSIGNED_BYTE, pixels);
}

void Texture::bind(unsigned int slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, this->tex);
}

void Texture::bind() {
	bind(0);
}

int Texture::get_atlas_slot() {
	return this->atlas_slot;
}

void Texture::set_filter(GLenum min_filter, GLenum mag_filter) {
	bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
}

int Texture::get_width() const { return width; }

int Texture::get_height() const { return height; }

const unsigned int Texture::get_tex() const { return tex; }

void Texture::render(Renderer *renderer, const glm::mat4 &model) {
	TextureInstance instance;
	instance.transform.x = model[3].x;
	instance.transform.y = model[3].y;
	instance.transform.z = glm::length(glm::vec2(model[0]));
	instance.transform.w = glm::length(glm::vec2(model[1]));
	instance.color = Color::white();
	instance.uv = get_atlas_uv(atlas_slot);
	instance.rotation = std::atan2(model[0].y, model[0].x);
	render_instanced(renderer, &instance, 1);
}

void Texture::check_instance_size(const TextureInstance *instances, int count) {
	int required_size = static_cast<int>(sizeof(TextureInstance) * count);

	if (required_size > this->instance_capacity) {
		glBufferData(GL_ARRAY_BUFFER, required_size, instances,
			GL_STREAM_DRAW);
		this->instance_capacity = required_size;
	}
	else {
		glBufferSubData(GL_ARRAY_BUFFER, 0, required_size, instances);
	}
}

void Texture::render_instanced(Renderer *renderer,
	const TextureInstance *instances, int count) {
	if (count <= 0)
		return;

	Shader *shader = renderer->get_shader();

	shader->bind();
	this->bind(0);

	glBindVertexArray(this->vao);
	glBindBuffer(GL_ARRAY_BUFFER, this->instance_vbo);

	check_instance_size(instances, count);

	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, count);
}
