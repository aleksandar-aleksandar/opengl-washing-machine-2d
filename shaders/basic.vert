#version 330 core

// input: one corner of a 1x1 square (from -0.5 to 0.5), sent from C++
layout(location = 0) in vec2 aPos;

// inputs from C++ (all in PIXELS)
uniform vec2 uPos;      // center of the shape
uniform vec2 uSize;     // width and height of the shape
uniform vec2 uScreen;   // width and height of the screen
uniform float uRotation; // rotation around the shape's center, in radians (0 = not rotated)

// output to the fragment shader: where inside the square this corner is (-0.5..0.5)
out vec2 vLocal;

void main() {
    vLocal = aPos;

    // 1. scale the 1x1 square to the right size (still centered on 0,0)
    vec2 p = aPos * uSize;

    // 2. rotate it around its center (the standard 2D rotation formula)
    float c = cos(uRotation);
    float s = sin(uRotation);
    p = vec2(p.x * c - p.y * s,
             p.x * s + p.y * c);

    // 3. move it to its position
    vec2 pixel = uPos + p;

    // 4. convert pixels (0..screen size) to OpenGL coordinates (-1..1)
    vec2 ndc = pixel / uScreen * 2.0 - 1.0;

    gl_Position = vec4(ndc, 0.0, 1.0);
}
