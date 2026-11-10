#include "texture.h"

#include <iostream>

// stb_image: loads PNG/JPG files. This #define must appear in exactly ONE .cpp file,
// it tells stb_image.h to include its actual code here (not just the declarations)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint loadTexture(const char* path) {
    stbi_set_flip_vertically_on_load(true);   // PNGs start at the TOP row, OpenGL expects the BOTTOM row first

    int width, height, channels;
    unsigned char* pixels = stbi_load(path, &width, &height, &channels, 4);  // 4 = always give us RGBA
    if (!pixels) {
        std::cout << "Could not load texture: " << path << "\n";
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);              // create a texture, put its ID in texture
    glBindTexture(GL_TEXTURE_2D, texture);   // select it
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);   // copy the pixels into GPU memory

    // when the texture is drawn bigger/smaller than the image: blend nearby pixels (smooth, not blocky)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // at the edges: don't wrap around to the other side of the image
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(pixels);   // the GPU has its own copy now, free ours
    return texture;
}

GLFWcursor* loadCursor(const char* path, int hotX, int hotY) {
    stbi_set_flip_vertically_on_load(false);   // cursors want the TOP row first (no flip, unlike textures)

    int width, height, channels;
    unsigned char* pixels = stbi_load(path, &width, &height, &channels, 4);
    if (!pixels) {
        std::cout << "Could not load cursor: " << path << "\n";
        return nullptr;
    }

    GLFWimage image;   // GLFW's simple image description: size + RGBA pixels
    image.width = width;
    image.height = height;
    image.pixels = pixels;
    GLFWcursor* cursor = glfwCreateCursor(&image, hotX, hotY);   // GLFW copies the pixels

    stbi_image_free(pixels);
    return cursor;
}
