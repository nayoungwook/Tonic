#pragma once

#include "engine/mathf.h"
#include "engine/texture.h"

class Sprite : public Texture {
public:
	using Texture::Texture;

	void fetch_atlas_data(const Vector &position_data,
		const Vector &size_data) {
		fetch_atlas(glm::vec4(position_data.x, position_data.y,
			size_data.x, size_data.y));
	}

	int get_atlas_index() { return get_atlas_slot(); }
	void set_atlas_index(int atlas_index) { set_atlas(atlas_index); }
	glm::vec2 get_texture_size() const {
		return glm::vec2(get_width(), get_height());
	}
	glm::vec4 get_atlas_uv_bounds(int atlas_index) const {
		return get_atlas_uv(atlas_index);
	}
};
