#pragma once

#include <memory>
#include <filesystem>
#include <string>
#include <vector>

#include <glm/glm.hpp>

class Texture;

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
		const std::string &text, const glm::vec4 &color,
		const glm::vec4 &outline_color = glm::vec4(39.0f / 255.0f,
			39.0f / 255.0f, 54.0f / 255.0f, 1.0f),
		float outline_width = 0.0f);
	TextMetrics measure_text(const std::string &text) const;

	float get_size() const;
	int get_last_width() const;
	int get_last_height() const;

private:
	struct CodepointLine {
		std::vector<unsigned int> codepoints;
	};

	static std::vector<CodepointLine> decode_lines(const std::string &text);
	std::filesystem::path resolve_font_path(const std::string &font_path) const;

	std::vector<unsigned char> font_data;
	void *font_info_storage = nullptr;
	float size = 16.0f;
	mutable int last_width = 0;
	mutable int last_height = 0;
};
