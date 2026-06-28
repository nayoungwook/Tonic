#include "engine/ttfont.h"

#include "engine/error.h"
#include "engine/texture.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

#include <SDL3_ttf/SDL_ttf.h>

namespace {
int ttf_init_count = 0;

unsigned char to_byte(float value) {
	return static_cast<unsigned char>(
		std::clamp(value, 0.0f, 1.0f) * 255.0f + 0.5f);
}

SDL_Color to_sdl_color(const Color &color) {
	return SDL_Color{
		to_byte(color.r),
		to_byte(color.g),
		to_byte(color.b),
		to_byte(color.a),
	};
}

bool acquire_ttf() {
	if (!TTF_Init()) {
		error(std::string("SDL3_ttf init failed: ") + SDL_GetError());
		return false;
	}

	++ttf_init_count;
	return true;
}

void release_ttf() {
	if (ttf_init_count <= 0)
		return;

	--ttf_init_count;
	TTF_Quit();
}

std::vector<std::string> split_lines(const std::string &text) {
	std::vector<std::string> lines;
	size_t start = 0;

	while (start <= text.size()) {
		size_t end = text.find('\n', start);
		if (end == std::string::npos) {
			lines.push_back(text.substr(start));
			break;
		}

		lines.push_back(text.substr(start, end - start));
		start = end + 1;
	}

	if (lines.empty())
		lines.emplace_back();
	return lines;
}

int measure_line_height(TTF_Font *font, const std::vector<std::string> &lines) {
	int line_height =
		std::max(1, std::max(TTF_GetFontLineSkip(font),
			TTF_GetFontHeight(font)));

	for (const std::string &line : lines) {
		if (line.empty())
			continue;

		int width = 0;
		int height = 0;
		if (!TTF_GetStringSize(font, line.c_str(), line.size(), &width,
				&height)) {
			error(std::string("Failed to measure text: ") +
				SDL_GetError());
		}
		line_height = std::max(line_height, height);
	}

	return line_height;
}

TTFont::TextMetrics measure_lines(TTF_Font *font,
	const std::vector<std::string> &lines) {
	TTFont::TextMetrics metrics;
	int line_height = measure_line_height(font, lines);
	int max_width = 0;

	for (const std::string &line : lines) {
		int width = 0;
		int height = 0;
		if (!line.empty() &&
			!TTF_GetStringSize(font, line.c_str(), line.size(), &width,
				&height)) {
			error(std::string("Failed to measure text: ") +
				SDL_GetError());
		}
		max_width = std::max(max_width, width);
	}

	metrics.width = std::max(1, max_width);
	metrics.height = std::max(1,
		line_height * static_cast<int>(std::max<size_t>(1, lines.size())));
	return metrics;
}

void blend_pixel(std::vector<unsigned char> &pixels, int width, int height,
	int x, int y, const unsigned char *src) {
	if (x < 0 || y < 0 || x >= width || y >= height || src[3] == 0)
		return;

	float src_a = src[3] / 255.0f;
	float dst_a = pixels[(y * width + x) * 4 + 3] / 255.0f;
	float out_a = src_a + dst_a * (1.0f - src_a);
	if (out_a <= 0.0f)
		return;

	int index = (y * width + x) * 4;
	float src_r = src[0] / 255.0f;
	float src_g = src[1] / 255.0f;
	float src_b = src[2] / 255.0f;
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

void blend_surface(std::vector<unsigned char> &pixels, int width, int height,
	SDL_Surface *surface, int dst_x, int dst_y) {
	if (surface == nullptr)
		return;

	SDL_Surface *rgba = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
	if (rgba == nullptr) {
		error(std::string("Failed to convert text surface: ") +
			SDL_GetError());
	}

	unsigned char *src_pixels =
		static_cast<unsigned char *>(rgba->pixels);
	for (int y = 0; y < rgba->h; ++y) {
		unsigned char *row = src_pixels + y * rgba->pitch;
		for (int x = 0; x < rgba->w; ++x) {
			blend_pixel(pixels, width, height, dst_x + x, dst_y + y,
				row + x * 4);
		}
	}

	SDL_DestroySurface(rgba);
}

SDL_Surface *render_line(TTF_Font *font, const std::string &line,
	const SDL_Color &color, int outline) {
	if (line.empty())
		return nullptr;

	if (!TTF_SetFontOutline(font, outline)) {
		error(std::string("Failed to set font outline: ") +
			SDL_GetError());
	}

	SDL_Surface *surface = TTF_RenderText_Blended(
		font, line.c_str(), line.size(), color);
	if (surface == nullptr) {
		error(std::string("Failed to render text: ") + SDL_GetError());
	}

	return surface;
}
}

TTFont::TTFont(const std::string &font_path, float font_size)
	: size(std::max(1.0f, font_size)) {
	ttf_initialized = acquire_ttf();

	std::filesystem::path resolved = resolve_font_path(font_path);
	font = TTF_OpenFont(resolved.string().c_str(), size);
	if (font == nullptr) {
		error("Failed to load font " + resolved.string() + ": " +
			SDL_GetError());
	}
}

TTFont::~TTFont() {
	if (font != nullptr) {
		TTF_CloseFont(font);
		font = nullptr;
	}

	if (ttf_initialized) {
		release_ttf();
		ttf_initialized = false;
	}
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

TTFont::TextMetrics TTFont::measure_text(const std::string &text) const {
	if (font == nullptr)
		return TextMetrics{};

	TTF_SetFontOutline(font, 0);
	return measure_lines(font, split_lines(text));
}

std::unique_ptr<Texture> TTFont::create_texture(const std::string &text,
	const Color &color, const Color &outline_color,
	float outline_width) {
	if (font == nullptr)
		return nullptr;

	TTF_SetFontOutline(font, 0);
	std::vector<std::string> lines = split_lines(text);
	TextMetrics metrics = measure_lines(font, lines);
	int outline = std::max(0, static_cast<int>(std::ceil(outline_width)));
	int width = std::max(1, metrics.width + outline * 2 + 4);
	int height = std::max(1, metrics.height + outline * 2 + 4);
	int line_height = measure_line_height(font, lines);
	std::vector<unsigned char> pixels(width * height * 4, 0);
	SDL_Color fill_color = to_sdl_color(color);
	SDL_Color stroke_color = to_sdl_color(outline_color);

	for (size_t line_index = 0; line_index < lines.size(); ++line_index) {
		const std::string &line = lines[line_index];
		int line_y = static_cast<int>(line_index) * line_height;

		if (outline > 0) {
			SDL_Surface *outline_surface =
				render_line(font, line, stroke_color, outline);
			blend_surface(pixels, width, height, outline_surface, 2,
				2 + line_y);
			if (outline_surface != nullptr)
				SDL_DestroySurface(outline_surface);
		}

		SDL_Surface *fill_surface = render_line(font, line, fill_color, 0);
		blend_surface(pixels, width, height, fill_surface, outline + 2,
			outline + 2 + line_y);
		if (fill_surface != nullptr)
			SDL_DestroySurface(fill_surface);
	}

	TTF_SetFontOutline(font, 0);

	last_width = width;
	last_height = height;
	auto texture = std::make_unique<Texture>(width, height, pixels.data());
	texture->set_filter(GL_LINEAR, GL_LINEAR);
	return texture;
}

float TTFont::get_size() const { return size; }

int TTFont::get_last_width() const { return last_width; }

int TTFont::get_last_height() const { return last_height; }
