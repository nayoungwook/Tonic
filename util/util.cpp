#include "engine/util.h"
#include "engine/error.h"

std::string read_file(const std::string &path) {
        std::ifstream file(path);
        if (!file.is_open()) {
                error("Failed to open shader file");
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
}
