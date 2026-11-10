#include <GL/glew.h>      // GLEW must be included BEFORE GLFW
#include <GLFW/glfw3.h>
#include <iostream>

#include "shader.h"    // createProgram
#include "texture.h"   // loadTexture, loadCursor
#include "draw.h"      // initDraw, beginDraw, drawShape...
#include "scene.h"     // the washing machine: initScene, handleMouse, updateScene, drawScene...

int main() {
    // graphics library framework

    if (!glfwInit()) {
        std::cout << "GLFW init failed\n";
        return -1;
    }

    // windoe settings
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // creating the windoews
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height,
                                          "Washing Machine", monitor, nullptr);
    if (!window) {
        std::cout << "Window creation failed\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);   // turn off V-Sync, so limiter decides the 75 i set FPS

    // setting up extensions wrangler in experiemtnal mode
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW init failed\n";
        glfwTerminate();
        return -1;
    }

    // load + compile our shaders, and set up drawing (uniforms + the 1x1 square)
    GLuint shaderProgram = createProgram("shaders/basic.vert", "shaders/basic.frag");
    initDraw(shaderProgram);

    // load the images (made by tools/make_textures.py and tools/make_nametag.py)
    GLuint tshirtTex  = loadTexture("textures/tshirt.png");
    GLuint pantsTex   = loadTexture("textures/pants.png");
    GLuint sockTex    = loadTexture("textures/sock.png");
    GLuint dressTex   = loadTexture("textures/dress.png");
    GLuint nametagTex = loadTexture("textures/nametag.png");
    GLuint arrowTex   = loadTexture("textures/arrow.png");

    // the mouse cursor: a fabric softener bottle (hotspot 16, 13 = the tip of the cap)
    GLFWcursor* cursor = loadCursor("textures/cursor_bottle.png", 16, 13);
    if (cursor) glfwSetCursor(window, cursor);

    // turn on transparency: mix each new pixel with what's already on screen, based on its alpha
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.15f, 0.25f, 0.35f, 1.0f);   // background color: R, G, B, A (0..1)

    const double targetFrameTime = 1.0 / 75.0;   // 75 FPS = 0.0133 seconds per frame

    // screen size in pixels (e.g. 1920 x 1080)
    float screenW = (float)mode->width;
    float screenH = (float)mode->height;

    // the washing machine + clothes (everything about them is in scene.h / scene.cpp)
    Scene scene;
    initScene(scene, screenW, screenH, tshirtTex, pantsTex, dressTex, sockTex, arrowTex);

    // render loop: every frame = 1. input, 2. update, 3. draw, 4. wait (frame limiter)
    while (!glfwWindowShouldClose(window)) {
        double frameStart = glfwGetTime();      // time when this frame started (in seconds)

        // --- 1. input ---
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        double mx, my;
        glfwGetCursorPos(window, &mx, &my);      // mouse position in pixels
        float mouseX = (float)mx;
        float mouseY = screenH - (float)my;      // GLFW's y starts at the TOP, ours at the BOTTOM, so flip it
        bool mouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        handleMouse(scene, mouseX, mouseY, mouseDown);

        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
            handleEnter(scene);

        // --- 2. update: move everything forward by one frame ---
        updateScene(scene, (float)targetFrameTime);

        // --- 3. draw ---
        glClear(GL_COLOR_BUFFER_BIT);
        beginDraw(screenW, screenH);

        drawScene(scene, mouseX, mouseY);

        // name tag in the bottom right corner, on top of everything (alpha 0.85 = a bit see-through)
        drawShape(screenW - 20 - 200, 20 + 50, 400, 100, 1.0f, 1.0f, 1.0f, 0.85f, false, nametagTex);

        glfwSwapBuffers(window);        // show what we just drew
        glfwPollEvents();               // event listerner

        // --- 4. frame limiter: wait until 1/75 of a second has passed since frameStart ---
        while (glfwGetTime() - frameStart < targetFrameTime) {
            // do nothing, just wait
        }
    }

    // --- cleanup ---
    GLuint textures[] = { tshirtTex, pantsTex, sockTex, dressTex, nametagTex, arrowTex };
    glDeleteTextures(6, textures);    // free the textures
    if (cursor) glfwDestroyCursor(cursor);
    cleanupDraw();                    // free the square (VAO + VBO)
    glDeleteProgram(shaderProgram);   // free the shader program from GPU memory
    glfwTerminate();
    return 0;
}
