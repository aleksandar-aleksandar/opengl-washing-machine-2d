#pragma once

#include <GL/glew.h>

// One piece of clothing: where it is, how big it is, what it looks like, and whether it's red
struct Cloth {
    float x, y;            // center, in pixels
    float width, height;   // in pixels
    GLuint texture;        // which image (t-shirt, pants...)
    bool isRed;
    bool inMachine;        // true once it's been dropped into the drum (then it can't be dragged)

    // where the cloth started (its "home" on the left side): the < > buttons stay here
    float homeX = 0, homeY = 0;

    // used while spinning
    float angle = 0;         // position on the circle, in radians (0 = right, PI/2 = top, PI = left, 3PI/2 = bottom)
    bool falling = false;    // true = falling straight down, false = riding along the circle
    float fallSpeed = 0;     // how fast it's falling right now, in pixels per second
    float rotation = 0;      // how much the cloth itself is turned (tumbling), in radians
};

// Is the point (px, py) inside the box with center (x, y) and this width and height?
bool isInsideBox(float px, float py, float x, float y, float width, float height);

// Is the point (px, py) inside this cloth's rectangle?
bool isInside(float px, float py, Cloth c);

// Is the point (px, py) on one of this cloth's < > buttons?
bool isOnArrow(float px, float py, Cloth c);

// Draws one cloth: its image, colored red or white.
// pink = how pink the WHITE clothes are: 0 = pure white, 1 = fully pink, 0.5 = halfway
void drawCloth(Cloth c, float pink = 0.0f);

// Draws the < and > buttons next to a cloth's home spot. They light up when the mouse is over them.
void drawArrows(Cloth c, GLuint arrowTex, float mouseX, float mouseY);
