#include "engine/shader.h"
#include "engine/util.h"

Shader::Shader(const std::string &frag_path, const std::string &vert_path) {
        load_shader(frag_path, vert_path);
}

void Shader::bind() { glUseProgram(this->program); }

const unsigned Shader::get_program() const { return program; }

unsigned Shader::get_uniform_location(const std::string &name) {
        unsigned uniform_loc = 0;
        if (this->uniform_cache.find(name) == this->uniform_cache.end()) {
                uniform_loc =
                    glGetUniformLocation(this->get_program(), name.c_str());
                this->uniform_cache.insert(std::make_pair(name, uniform_loc));
        }

        return uniform_loc = uniform_cache.find(name)->second;
}

void Shader::load_shader(const std::string &frag_path,
                         const std::string &vert_path) {
        std::string vert_str = read_file(vert_path);
        const char *vert = vert_str.c_str();

        std::string frag_str = read_file(frag_path);
        const char *frag = frag_str.c_str();

        unsigned vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vert, nullptr);
        glCompileShader(vertex_shader);
        check_shader_compile(vertex_shader, frag_path);

        unsigned fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &frag, nullptr);
        glCompileShader(fragment_shader);
        check_shader_compile(fragment_shader, vert_path);

        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);
}

void Shader::check_shader_compile(unsigned shader, const std::string &name) {
        int success;
        char log[1024];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success) {
                glGetShaderInfoLog(shader, 1024, nullptr, log);
                std::cerr
                    << "ERROR::SHADER_COMPILATION_ERROR " << name << "\n"
                    << log
                    << "\n -- "
                       "--------------------------------------------------- -- "
                    << std::endl;
        }
}