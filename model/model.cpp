#include "model.h"
#include <error.h>
#include "util.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

#include <memory>
#include <stdexcept>

Model::Model(const std::string &path) {
	load_model(path);
}

void Model::load_model(const std::string &path) {
	const std::string full_path = resolve_resource_path(path);
	size_t size = 0;
	std::unique_ptr<void, decltype(&SDL_free)> data(
		SDL_LoadFile(full_path.c_str(), &size), SDL_free);

	if (!data)
		error("Failed to load model (obj file) : " + path);

	const std::string text(static_cast<const char *>(data.get()), size);
	data.reset();

	tinyobj::ObjReaderConfig config;
	config.triangulate = true;
	config.vertex_color = false;
	tinyobj::ObjReader reader;

	if (!reader.ParseFromString(text, "", config))
		error("Failed to parse model (obj file) : " + path);

	const auto &positions = reader.GetAttrib().vertices;
	vertices.assign(positions.begin(), positions.end());
	std::size_t index_count = 0;

	for (const auto &shape : reader.GetShapes())
		index_count += shape.mesh.indices.size();

	indices.reserve(index_count);

	for (const auto &shape : reader.GetShapes()) {
		for (const auto &index : shape.mesh.indices) {
			if (index.vertex_index < 0 ||
				static_cast<std::size_t>(index.vertex_index) >= vertices.size() / 3)
				error("OBJ vertex index out of range : " + full_path);

			indices.push_back(static_cast<unsigned>(index.vertex_index));
		}
	}

	if (vertices.empty() || indices.empty())
		error("OBJ has no triangle geometry: " + full_path);
}

const std::vector<float> &Model::get_vertices() const { return vertices; }
const std::vector<unsigned> &Model::get_indices() const { return indices; }
