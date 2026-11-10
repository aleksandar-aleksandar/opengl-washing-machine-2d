#pragma once   // "only include this file once", even if several files #include it

#include <GL/glew.h>

// Reads a vertex shader and a fragment shader file, compiles both on the GPU
// and links them into one "shader program". Returns the program's ID.
GLuint createProgram(const char* vertPath, const char* fragPath);
