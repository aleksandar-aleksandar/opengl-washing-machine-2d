#version 330 core

// input from the vertex shader: where inside the square this pixel is (-0.5..0.5)
in vec2 vLocal;

// output: the final color of this pixel
out vec4 FragColor;

// inputs from C++
uniform vec4 uColor;        // the color to use (with a texture: the image gets tinted by it)
uniform int uCircle;        // 1 = draw as a circle, 0 = draw as a rectangle
uniform int uUseTexture;    // 1 = draw the texture, 0 = just the flat color
uniform sampler2D uTexture; // the image to read from
uniform float uHole;        // circles only: radius of a hole in the middle (0 = no hole, used for rings)
uniform float uCutTop;      // how much to cut off the top (0 = nothing, 1 = everything), used for the water level

void main() {
    // cut off the top part: the square goes from -0.5 (bottom) to 0.5 (top)
    if (vLocal.y > 0.5 - uCutTop)
        discard;

    // length(vLocal) = distance from the center of the square
    // the square goes from -0.5 to 0.5, so the biggest circle that fits has radius 0.5
    if (uCircle == 1) {
        float dist = length(vLocal);
        if (dist > 0.5 || dist < uHole)
            discard;   // outside the circle, or inside the hole: don't draw this pixel
    }

    if (uUseTexture == 1) {
        // texture coordinates go from 0 to 1, our square from -0.5 to 0.5, so add 0.5
        vec2 uv = vLocal + 0.5;
        // read the image's color at this spot, and tint it: white * uColor = uColor
        FragColor = texture(uTexture, uv) * uColor;
    } else {
        FragColor = uColor;
    }
}
