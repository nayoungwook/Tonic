#pragma once

#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <stb/stb_image.h>

struct Rect {
        float left;
        float right;
        float bottom;
        float top;
};

std::string read_file(const std::string &path);
