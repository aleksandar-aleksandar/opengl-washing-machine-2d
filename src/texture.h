#pragma once

#include <GL/glew.h>      // GLEW must be included BEFORE GLFW
#include <GLFW/glfw3.h>

// Loads a PNG file into GPU memory as a texture, returns its ID (0 = failed)
GLuint loadTexture(const char* path);

// Loads a PNG and turns it into a mouse cursor (nullptr = failed).
// hotX, hotY = which pixel of the image is the "click point" (counted from the image's top left)
GLFWcursor* loadCursor(const char* path, int hotX, int hotY);
