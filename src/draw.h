#pragma once

#include <GL/glew.h>

// Everything is drawn from ONE 1x1 square. The shader scales, rotates and moves it,
// and colors / textures / cuts it into circles, rings, etc. based on uniforms.

// Call once at the start: finds the shader's uniforms and creates the 1x1 square (VAO + VBO)
void initDraw(GLuint shaderProgram);

// Call at the start of each frame's drawing: use our shader + square, tell the shader the screen size
void beginDraw(float screenW, float screenH);

// Call once at the end: frees the square from GPU memory
void cleanupDraw();

// Draws one shape. Position (center) and size are in pixels.
// texture = 0 means "no texture, just the flat color"
void drawShape(float x, float y, float width, float height,
               float r, float g, float b, float a, bool isCircle, GLuint texture = 0);

// Draws a ring (a circle with a hole in the middle), e.g. the door frame.
// holeFraction = hole size compared to the whole ring (0.8 = the hole is 80% as big).
// If width != height, the ring is squashed into an oval (used for the swinging door).
void drawRing(float x, float y, float width, float height, float holeFraction,
              float r, float g, float b, float a);

// Draws the water in the drum: a see-through blue circle, cut off at the water level.
// level = 0 (empty) .. 1 (full to the top)
void drawWater(float x, float y, float size, float level);

// Draws one digit like on a digital clock (7 segments). digit = 0..9, or anything else for "-"
void drawDigit(float x, float y, float height, int digit);

// Rotation for the NEXT shapes, in radians (remember to set it back to 0 afterwards)
void setRotation(float radians);
