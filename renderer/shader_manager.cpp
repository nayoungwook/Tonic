#include "engine/shader_manager.h"

#include "engine/error.h"

void ShaderManager::fetch_shader(const std::string &name, Shader *shader) {
        this->shaders.insert(std::make_pair(name, shader));
}

Shader *ShaderManager::get_shader(const std::string &name) {
        Shader *result = nullptr;
        std::unordered_map<std::string, Shader *>::iterator shader_it =
            this->shaders.find(name);

        if (shader_it == this->shaders.end()) {
                error("Failed to find shader " + name);
        }

        return shader_it->second;
}