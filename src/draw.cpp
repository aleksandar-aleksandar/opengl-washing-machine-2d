#include "draw.h"

// These live only inside this file ("static"): the rest of the program just calls drawShape etc.
static GLuint program;                // our shader program
static GLuint VAO, VBO;               // the 1x1 square
static GLint colorLoc, posLoc, sizeLoc, screenLoc, circleLoc, useTextureLoc,
             holeLoc, rotationLoc, cutTopLoc;   // where each uniform lives in the shader

void initDraw(GLuint shaderProgram) {
    program = shaderProgram;

    // find where each uniform lives inside our shader program (do this once)
    colorLoc      = glGetUniformLocation(program, "uColor");
    posLoc        = glGetUniformLocation(program, "uPos");
    sizeLoc       = glGetUniformLocation(program, "uSize");
    screenLoc     = glGetUniformLocation(program, "uScreen");
    circleLoc     = glGetUniformLocation(program, "uCircle");
    useTextureLoc = glGetUniformLocation(program, "uUseTexture");
    holeLoc       = glGetUniformLocation(program, "uHole");
    rotationLoc   = glGetUniformLocation(program, "uRotation");
    cutTopLoc     = glGetUniformLocation(program, "uCutTop");

    // a 1x1 square = 2 triangles = 6 corners, each corner is (x, y)
    // (the shader scales it to any size and moves it anywhere)
    float vertices[] = {
        // triangle 1
        -0.5f, -0.5f,   // bottom left
         0.5f, -0.5f,   // bottom right
         0.5f,  0.5f,   // top right
        // triangle 2
        -0.5f, -0.5f,   // bottom left
         0.5f,  0.5f,   // top right
        -0.5f,  0.5f,   // top left
    };

    glGenVertexArrays(1, &VAO);   // create a VAO, put its ID in VAO
    glGenBuffers(1, &VBO);        // create a VBO, put its ID in VBO

    glBindVertexArray(VAO);       // "the following settings belong to this VAO"

    glBindBuffer(GL_ARRAY_BUFFER, VBO);                                        // select our VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // copy corners into GPU memory

    // how to read the VBO: location 0, 2 floats per corner, corners are 2 floats apart, start at 0
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);  // turn on input slot 0 (aPos in basic.vert)

    glBindVertexArray(0);          // done setting up, unselect the VAO
}

void beginDraw(float screenW, float screenH) {
    glUseProgram(program);                       // use our shaders
    glUniform2f(screenLoc, screenW, screenH);    // screen size
    glBindVertexArray(VAO);                      // use our square (for every shape after this)
}

void cleanupDraw() {
    glDeleteVertexArrays(1, &VAO);    // free the VAO
    glDeleteBuffers(1, &VBO);         // free the VBO
}

void drawShape(float x, float y, float width, float height,
               float r, float g, float b, float a, bool isCircle, GLuint texture) {
    if (texture != 0) {
        glBindTexture(GL_TEXTURE_2D, texture);   // the shader will read from this texture
        glUniform1i(useTextureLoc, 1);
    } else {
        glUniform1i(useTextureLoc, 0);
    }
    glUniform1i(circleLoc, isCircle ? 1 : 0);
    glUniform4f(colorLoc, r, g, b, a);
    glUniform2f(posLoc, x, y);
    glUniform2f(sizeLoc, width, height);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void drawRing(float x, float y, float width, float height, float holeFraction,
              float r, float g, float b, float a) {
    glUniform1f(holeLoc, holeFraction * 0.5f);   // hole radius, in the square's -0.5..0.5 units
    drawShape(x, y, width, height, r, g, b, a, true);
    glUniform1f(holeLoc, 0.0f);                  // back to "no hole" for the next shapes
}

void drawWater(float x, float y, float size, float level) {
    glUniform1f(cutTopLoc, 1.0f - level);        // cut off everything above the water line
    drawShape(x, y, size, size, 0.30f, 0.55f, 0.95f, 0.30f, true);
    glUniform1f(cutTopLoc, 0.0f);                // back to "no cut" for the next shapes
}

void setRotation(float radians) {
    glUniform1f(rotationLoc, radians);
}

// Draws one digit like on a digital clock: 7 segments (a-g), each one a thin rectangle.
//
//     aaa
//    f   b
//     ggg
//    e   c
//     ddd
void drawDigit(float x, float y, float height, int digit) {
    // which segments are lit for each digit:  a  b  c  d  e  f  g   (1 = lit)
    const int segments[11][7] = {
                                             { 1, 1, 1, 1, 1, 1, 0 },   // 0
                                             { 0, 1, 1, 0, 0, 0, 0 },   // 1
                                             { 1, 1, 0, 1, 1, 0, 1 },   // 2
                                             { 1, 1, 1, 1, 0, 0, 1 },   // 3
                                             { 0, 1, 1, 0, 0, 1, 1 },   // 4
                                             { 1, 0, 1, 1, 0, 1, 1 },   // 5
                                             { 1, 0, 1, 1, 1, 1, 1 },   // 6
                                             { 1, 1, 1, 0, 0, 0, 0 },   // 7
                                             { 1, 1, 1, 1, 1, 1, 1 },   // 8
                                             { 1, 1, 1, 1, 0, 1, 1 },   // 9
                                             { 0, 0, 0, 0, 0, 0, 1 },   // dash
    };
    int row = (digit >= 0 && digit <= 9) ? digit : 10;

    float h = height;
    float w = h * 0.55f;   // digit width
    float t = h * 0.12f;   // segment thickness

    // center and size of each segment, relative to the digit's center
    //                a            b            c             d             e             f             g
    float segX[7] = { 0,           w/2 - t/2,   w/2 - t/2,    0,            -w/2 + t/2,   -w/2 + t/2,   0 };
    float segY[7] = { h/2 - t/2,   h/4,         -h/4,         -h/2 + t/2,   -h/4,         h/4,          0 };
    float segW[7] = { w,           t,           t,            w,            t,            t,            w };
    float segH[7] = { t,           h/2,         h/2,          t,            h/2,          h/2,          t };

    for (int s = 0; s < 7; s++) {
        float alpha = segments[row][s] ? 1.0f : 0.12f;   // unlit segments stay faintly visible, like a real display
        drawShape(x + segX[s], y + segY[s], segW[s], segH[s], 0.3f, 1.0f, 0.5f, alpha, false);
    }
}
