#include "texture.h"
#include "camera.h"
#include "error.h"
#include "renderer.h"
#include "shader.h"
#include "util.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
// Resource paths from util.h use UTF-8 on Windows as well.
#define STBI_WINDOWS_UTF8
#include <stb/stb_image.h>

namespace {
std::string resolve_resource_path(const std::string &path) {
	// Textures can be loaded from absolute paths, cwd-relative paths, the
	// configured resource root, or a resources folder above the executable.
	if (is_absolute_path(path) && file_exists(path))
		return path;

	std::string cwd = current_working_directory();
	std::array<std::string, 4> direct_candidates = {
		join_path(cwd, path),
		join_path(join_path(cwd, "resources"), path),
#ifdef ENGINE_RESOURCE_ROOT
		join_path(ENGINE_RESOURCE_ROOT, path),
#else
		std::string(),
#endif
		join_path("resources", path),
	};

	for (const auto &candidate : direct_candidates) {
		if (!candidate.empty() && file_exists(candidate))
			return candidate;
	}

	for (std::string probe = cwd; !probe.empty();) {
		std::string candidate = join_path(join_path(probe, "resources"), path);
		if (file_exists(candidate))
			return candidate;

		std::string parent = parent_path(probe);
		if (probe == parent)
			break;
		probe = parent;
	}

	return join_path(cwd, path);
}

unsigned char color_channel_to_byte(float value) {
	return static_cast<unsigned char>(std::round(
		std::max(0.0f, std::min(value, 1.0f)) * 255.0f));
}
}

Texture::Texture(const std::string &path) : path(path) {
	// File-backed textures allocate GL state first, then upload decoded pixels.
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
	// Texture owns dynamic vertex buffers for the batch renderer's quad data
	// layout: position, UV, and packed color.
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	constexpr GLsizei STRIDE = sizeof(BatchVertex);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE,
		(void *)offsetof(BatchVertex, x));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, STRIDE,
		(void *)offsetof(BatchVertex, u));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, STRIDE,
		(void *)offsetof(BatchVertex, r));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

void Texture::load_texture() {
	// stb_image decodes into CPU memory; upload_pixels transfers it to GL and
	// the CPU copy is released immediately after.
	std::string full_path = resolve_resource_path(path);

	unsigned char *data =
		stbi_load(full_path.c_str(), &width, &height, &channels, 0);

	if (!data) {
		error("Failed to load texture " + full_path);
		return;
	}

	GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
	upload_pixels(data, format);

	stbi_image_free(data);
}

void Texture::upload_pixels(const unsigned char *pixels, GLenum format) {
	// Atlas and framebuffer sampling expect pixel-art friendly nearest filters
	// and clamped edges unless a higher-level shader decides otherwise.
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

void Texture::render_instanced(Renderer *renderer,
	const TextureInstance *instances, int count) {
	if (count <= 0)
		return;

	Shader *shader = renderer->get_shader();

	shader->bind();
	this->bind(0);

	constexpr int VERTICES_PER_SPRITE = 4;
	constexpr int INDICES_PER_SPRITE = 6;
	batch_vertices.resize(static_cast<std::size_t>(count) *
		VERTICES_PER_SPRITE);
	batch_indices.resize(static_cast<std::size_t>(count) * INDICES_PER_SPRITE);

	for (int i = 0; i < count; ++i) {
		const TextureInstance &instance = instances[i];
		const float half_width = instance.transform.z * 0.5f;
		const float half_height = instance.transform.w * 0.5f;
		const float cosine = std::cos(instance.rotation);
		const float sine = std::sin(instance.rotation);
		const float local_x[VERTICES_PER_SPRITE] = {
			-half_width, half_width, half_width, -half_width };
		const float local_y[VERTICES_PER_SPRITE] = {
			half_height, half_height, -half_height, -half_height };
		const float tex_u[VERTICES_PER_SPRITE] = {
			instance.uv.x, instance.uv.z, instance.uv.z, instance.uv.x };
		const float tex_v[VERTICES_PER_SPRITE] = {
			instance.uv.y, instance.uv.y, instance.uv.w, instance.uv.w };
		const unsigned char red = color_channel_to_byte(instance.color.r);
		const unsigned char green = color_channel_to_byte(instance.color.g);
		const unsigned char blue = color_channel_to_byte(instance.color.b);
		const unsigned char alpha = color_channel_to_byte(instance.color.a);

		for (int vertex = 0; vertex < VERTICES_PER_SPRITE; ++vertex) {
			const float x = local_x[vertex];
			const float y = local_y[vertex];
			BatchVertex &output = batch_vertices[
				static_cast<std::size_t>(i) * VERTICES_PER_SPRITE + vertex];
			output.x = instance.transform.x + x * cosine - y * sine;
			output.y = instance.transform.y + x * sine + y * cosine;
			output.z = 0.0f;
			output.u = tex_u[vertex];
			output.v = tex_v[vertex];
			output.r = red;
			output.g = green;
			output.b = blue;
			output.a = alpha;
		}

		const unsigned base = static_cast<unsigned>(i * VERTICES_PER_SPRITE);
		unsigned *indices = batch_indices.data() +
			static_cast<std::size_t>(i) * INDICES_PER_SPRITE;
		indices[0] = base;
		indices[1] = base + 1;
		indices[2] = base + 2;
		indices[3] = base + 2;
		indices[4] = base + 3;
		indices[5] = base;
	}

	shader->upload_mat4("uModel", glm::mat4(1.0f));
	glBindVertexArray(this->vao);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferData(GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(batch_vertices.size() * sizeof(BatchVertex)),
		batch_vertices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(batch_indices.size() * sizeof(unsigned)),
		batch_indices.data(), GL_DYNAMIC_DRAW);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(batch_indices.size()),
		GL_UNSIGNED_INT, nullptr);
}
