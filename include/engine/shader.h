#pragma once

#include <GL/glew.h>
#include <SDL/SDL.h>

#include <iostream>
#include <unordered_map>

class Shader {
public:
	Shader(const std::string &frag_path, const std::string &vert_path);

	void bind();

	const unsigned get_program() const;
	unsigned get_uniform_location(const std::string &name);

private:

	std::unordered_map<std::string, unsigned int> uniform_cache;

	void check_shader_compile(unsigned id, const std::string &name);
	unsigned fragment, vertex, program;
	void load_shader(const std::string &frag_path, const std::string &vert_path);
};