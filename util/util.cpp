#include "engine/util.h"
#include "engine/error.h"

#include <filesystem>

std::string read_file(const std::string &path) {
        std::filesystem::path resolved_path(path);
        if (!std::filesystem::exists(resolved_path)) {
#ifdef ENGINE_SHADER_ROOT
                std::filesystem::path shader_root(ENGINE_SHADER_ROOT);
                std::filesystem::path requested(path);
                if (requested.has_parent_path() &&
                    requested.parent_path().filename() == "shaders") {
                        requested = requested.filename();
                }
                std::filesystem::path candidate = shader_root / requested;
                if (std::filesystem::exists(candidate)) {
                        resolved_path = candidate;
                }
#endif
        }

        std::ifstream file(resolved_path);
        if (!file.is_open()) {
                error("Failed to open file : " + path);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
}
