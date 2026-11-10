#include "shader.h"

#include <iostream>
#include <fstream>   // for reading files
#include <sstream>   // for turning a file into one string
#include <string>

// Reads a whole text file and returns its contents as one string
// ("static" = this function is only used inside this file, other files can't see it)
static std::string readFile(const char* path) {
    std::ifstream file(path);
    if (!file) {
        std::cout << "Could not open file: " << path << "\n";
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();   // copy the whole file into the buffer
    return buffer.str();
}

// Reads one shader file and compiles it on the GPU
static GLuint compileShader(GLenum type, const char* path) {
    std::string code = readFile(path);
    const char* source = code.c_str();

    GLuint shader = glCreateShader(type);         // create an empty shader
    glShaderSource(shader, 1, &source, nullptr);  // give it the GLSL code
    glCompileShader(shader);                      // compile it

    // check for errors (e.g. a typo in the .vert/.frag file)
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cout << "Shader error in " << path << ":\n" << log << "\n";
    }
    return shader;
}

// Compiles both shaders and links them into one "shader program"
GLuint createProgram(const char* vertPath, const char* fragPath) {
    GLuint vertexShader   = compileShader(GL_VERTEX_SHADER, vertPath);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragPath);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);   // connect them together

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::cout << "Program link error:\n" << log << "\n";
    }

    // the program now has its own copy, so the separate shaders aren't needed
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}
