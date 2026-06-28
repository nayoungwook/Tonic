#pragma once

#include <memory>
#include <filesystem>
#include <string>

#include "engine/color.h"

class Texture;
struct TTF_Font;

class TTFont {
public:
	struct TextMetrics {
		int width = 0;
		int height = 0;
	};

	TTFont(const std::string &font_path, float font_size);
	~TTFont();
	TTFont(const TTFont &) = delete;
	TTFont &operator=(const TTFont &) = delete;

	std::unique_ptr<Texture> create_texture(
		const std::string &text, const Color &color,
		const Color &outline_color = Color::outline(),
		float outline_width = 0.0f);
	TextMetrics measure_text(const std::string &text) const;

	float get_size() const;
	int get_last_width() const;
	int get_last_height() const;

private:
	std::filesystem::path resolve_font_path(const std::string &font_path) const;

	TTF_Font *font = nullptr;
	bool ttf_initialized = false;
	float size = 16.0f;
	mutable int last_width = 0;
	mutable int last_height = 0;
};
