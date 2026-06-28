#include "engine/ttfont.h"

#include "engine/error.h"
#include "engine/texture.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

namespace {
unsigned char to_byte(float value) {
	return static_cast<unsigned char>(
		std::clamp(value, 0.0f, 1.0f) * 255.0f + 0.5f);
}

void blend_pixel(std::vector<unsigned char> &pixels, int width, int height,
	int x, int y, const glm::vec4 &color, unsigned char alpha) {
	if (x < 0 || y < 0 || x >= width || y >= height || alpha == 0)
		return;

	float src_a = (alpha / 255.0f) * std::clamp(color.a, 0.0f, 1.0f);
	int index = (y * width + x) * 4;
	float dst_a = pixels[index + 3] / 255.0f;
	float out_a = src_a + dst_a * (1.0f - src_a);
	if (out_a <= 0.0f)
		return;

	float src_r = std::clamp(color.r, 0.0f, 1.0f);
	float src_g = std::clamp(color.g, 0.0f, 1.0f);
	float src_b = std::clamp(color.b, 0.0f, 1.0f);
	float dst_r = pixels[index] / 255.0f;
	float dst_g = pixels[index + 1] / 255.0f;
	float dst_b = pixels[index + 2] / 255.0f;

	pixels[index] = to_byte((src_r * src_a + dst_r * dst_a *
		(1.0f - src_a)) / out_a);
	pixels[index + 1] = to_byte((src_g * src_a + dst_g * dst_a *
		(1.0f - src_a)) / out_a);
	pixels[index + 2] = to_byte((src_b * src_a + dst_b * dst_a *
		(1.0f - src_a)) / out_a);
	pixels[index + 3] = to_byte(out_a);
}

void draw_glyph(stbtt_fontinfo *font, std::vector<unsigned char> &pixels,
	int width, int height, unsigned int codepoint, float scale, int pen_x,
	int baseline, const glm::vec4 &color) {
	int glyph_width = 0;
	int glyph_height = 0;
	int xoff = 0;
	int yoff = 0;
	unsigned char *bitmap = stbtt_GetCodepointBitmap(
		font, 0, scale, static_cast<int>(codepoint), &glyph_width,
		&glyph_height, &xoff, &yoff);

	if (bitmap == nullptr)
		return;

	for (int y = 0; y < glyph_height; ++y) {
		for (int x = 0; x < glyph_width; ++x) {
			unsigned char alpha = bitmap[y * glyph_width + x];
			blend_pixel(pixels, width, height, pen_x + xoff + x,
				baseline + yoff + y, color, alpha);
		}
	}

	stbtt_FreeBitmap(bitmap, nullptr);
}
}

TTFont::TTFont(const std::string &font_path, float font_size)
	: size(std::max(1.0f, font_size)) {
	std::filesystem::path resolved = resolve_font_path(font_path);
	std::ifstream file(resolved, std::ios::binary);
	if (!file) {
		error("Failed to load font " + resolved.string());
		return;
	}

	font_data.assign(std::istreambuf_iterator<char>(file),
		std::istreambuf_iterator<char>());
	font_info_storage = new stbtt_fontinfo();

	if (font_data.empty() ||
		!stbtt_InitFont(static_cast<stbtt_fontinfo *>(font_info_storage),
			font_data.data(), 0)) {
		error("Failed to parse font " + resolved.string());
	}
}

TTFont::~TTFont() {
	delete static_cast<stbtt_fontinfo *>(font_info_storage);
	font_info_storage = nullptr;
}

std::filesystem::path TTFont::resolve_font_path(
	const std::string &font_path) const {
	std::filesystem::path requested(font_path);
	if (requested.is_absolute() && std::filesystem::exists(requested))
		return requested;

	std::filesystem::path cwd = std::filesystem::current_path();
	std::array<std::filesystem::path, 5> candidates = {
		cwd / requested,
		cwd / "resources" / requested,
		cwd / "resources" / "font" / requested,
		cwd / "Toffee" / "engineResources" / "font" / requested,
#ifdef ENGINE_RESOURCE_ROOT
		std::filesystem::path(ENGINE_RESOURCE_ROOT) / requested,
#else
		std::filesystem::path(),
#endif
	};

	for (const auto &candidate : candidates) {
		if (!candidate.empty() && std::filesystem::exists(candidate))
			return candidate;
	}

	return cwd / requested;
}

std::vector<TTFont::CodepointLine> TTFont::decode_lines(
	const std::string &text) {
	std::vector<CodepointLine> lines(1);

	for (size_t i = 0; i < text.size();) {
		unsigned char c = static_cast<unsigned char>(text[i]);
		unsigned int codepoint = 0;
		size_t advance = 1;

		if (c == '\n') {
			lines.emplace_back();
			++i;
			continue;
		}
		if ((c & 0x80) == 0) {
			codepoint = c;
		} else if ((c & 0xE0) == 0xC0 && i + 1 < text.size()) {
			codepoint = ((c & 0x1F) << 6) |
				(static_cast<unsigned char>(text[i + 1]) & 0x3F);
			advance = 2;
		} else if ((c & 0xF0) == 0xE0 && i + 2 < text.size()) {
			codepoint = ((c & 0x0F) << 12) |
				((static_cast<unsigned char>(text[i + 1]) & 0x3F) << 6) |
				(static_cast<unsigned char>(text[i + 2]) & 0x3F);
			advance = 3;
		} else if ((c & 0xF8) == 0xF0 && i + 3 < text.size()) {
			codepoint = ((c & 0x07) << 18) |
				((static_cast<unsigned char>(text[i + 1]) & 0x3F) << 12) |
				((static_cast<unsigned char>(text[i + 2]) & 0x3F) << 6) |
				(static_cast<unsigned char>(text[i + 3]) & 0x3F);
			advance = 4;
		} else {
			codepoint = '?';
		}

		lines.back().codepoints.push_back(codepoint);
		i += advance;
	}

	return lines;
}

TTFont::TextMetrics TTFont::measure_text(const std::string &text) const {
	TextMetrics metrics;
	if (font_info_storage == nullptr)
		return metrics;

	auto *font = static_cast<stbtt_fontinfo *>(font_info_storage);
	float scale = stbtt_ScaleForPixelHeight(font, size);
	int ascent = 0;
	int descent = 0;
	int line_gap = 0;
	stbtt_GetFontVMetrics(font, &ascent, &descent, &line_gap);
	int line_height = std::max(1,
		static_cast<int>(std::ceil((ascent - descent + line_gap) * scale)));

	std::vector<CodepointLine> lines = decode_lines(text);
	int max_width = 0;
	for (const CodepointLine &line : lines) {
		float x = 0.0f;
		for (size_t i = 0; i < line.codepoints.size(); ++i) {
			int advance = 0;
			int left_bearing = 0;
			stbtt_GetCodepointHMetrics(font,
				static_cast<int>(line.codepoints[i]), &advance,
				&left_bearing);
			x += advance * scale;
			if (i + 1 < line.codepoints.size()) {
				x += stbtt_GetCodepointKernAdvance(font,
					static_cast<int>(line.codepoints[i]),
					static_cast<int>(line.codepoints[i + 1])) * scale;
			}
		}
		max_width = std::max(max_width, static_cast<int>(std::ceil(x)));
	}

	metrics.width = std::max(1, max_width);
	metrics.height = std::max(1, line_height *
		static_cast<int>(std::max<size_t>(1, lines.size())));
	return metrics;
}

std::unique_ptr<Texture> TTFont::create_texture(const std::string &text,
	const glm::vec4 &color, const glm::vec4 &outline_color,
	float outline_width) {
	if (font_info_storage == nullptr)
		return nullptr;

	auto *font = static_cast<stbtt_fontinfo *>(font_info_storage);
	float scale = stbtt_ScaleForPixelHeight(font, size);
	int ascent = 0;
	int descent = 0;
	int line_gap = 0;
	stbtt_GetFontVMetrics(font, &ascent, &descent, &line_gap);

	int baseline_offset = static_cast<int>(std::ceil(ascent * scale));
	int line_height = std::max(1,
		static_cast<int>(std::ceil((ascent - descent + line_gap) * scale)));
	int outline = std::max(0, static_cast<int>(std::ceil(outline_width)));
	TextMetrics metrics = measure_text(text);
	int width = std::max(1, metrics.width + outline * 2 + 4);
	int height = std::max(1, metrics.height + outline * 2 + 4);
	std::vector<unsigned char> pixels(width * height * 4, 0);
	std::vector<CodepointLine> lines = decode_lines(text);

	for (size_t line_index = 0; line_index < lines.size(); ++line_index) {
		const CodepointLine &line = lines[line_index];
		int pen_x = outline + 2;
		int baseline = outline + 2 + baseline_offset +
			static_cast<int>(line_index) * line_height;

		for (size_t i = 0; i < line.codepoints.size(); ++i) {
			unsigned int codepoint = line.codepoints[i];

			if (outline > 0) {
				for (int oy = -outline; oy <= outline; ++oy) {
					for (int ox = -outline; ox <= outline; ++ox) {
						if (ox == 0 && oy == 0)
							continue;
						if (ox * ox + oy * oy > outline * outline)
							continue;
						draw_glyph(font, pixels, width, height, codepoint,
							scale, pen_x + ox, baseline + oy,
							outline_color);
					}
				}
			}

			draw_glyph(font, pixels, width, height, codepoint, scale, pen_x,
				baseline, color);

			int advance = 0;
			int left_bearing = 0;
			stbtt_GetCodepointHMetrics(font, static_cast<int>(codepoint),
				&advance, &left_bearing);
			pen_x += static_cast<int>(std::round(advance * scale));
			if (i + 1 < line.codepoints.size()) {
				pen_x += static_cast<int>(std::round(
					stbtt_GetCodepointKernAdvance(font,
						static_cast<int>(codepoint),
						static_cast<int>(line.codepoints[i + 1])) * scale));
			}
		}
	}

	last_width = width;
	last_height = height;
	auto texture = std::make_unique<Texture>(width, height, pixels.data());
	texture->set_filter(GL_LINEAR, GL_LINEAR);
	return texture;
}

float TTFont::get_size() const { return size; }

int TTFont::get_last_width() const { return last_width; }

int TTFont::get_last_height() const { return last_height; }
