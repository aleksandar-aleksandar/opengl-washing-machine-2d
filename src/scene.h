#pragma once

#include "cloth.h"

// --- the stages of the program (the "state machine") ---
enum State {
    LOADING,        // door open, you can drag clothes into the drum
    DOOR_CLOSING,   // ENTER was pressed, the door is swinging shut over the drum
    SPIN_UP,        // door closed, the clothes spin faster and faster, water fills up
    SPINNING,       // top speed reached, keep spinning for 20 seconds
    STOPPING,       // spinning stopped: water drains, clothes in the air land
    DOOR_OPENING,   // the door swings back open
    DONE,           // all finished, the only thing left is ESC to exit
};

const int CLOTH_COUNT = 4;   // 3 white + 1 red

// Everything about the washing machine and the clothes, in one place
struct Scene {
    // --- layout (all in pixels), set in initScene ---
    float screenW, screenH;
    float machineX, machineY;        // center of the machine body
    float drumX, drumY, drumRadius;  // the drum
    float pathRadius;                // the circle the clothes ride on while spinning
    float doorRadius, glassRadius;   // door frame and its glass
    float hingeX;                    // the door turns around this x

    // --- what's happening right now ---
    State state = LOADING;
    float doorProgress = 0.0f;   // 0 = fully open, 1 = fully closed
    float doorX = 0.0f;          // the door's center right now
    float doorSquash = 1.0f;     // how wide the door looks: 1 = facing us, 0 = edge-on (mid-swing)
    float spinSpeed = 0.0f;      // current speed, in radians per second
    float spinTimer = 0.0f;      // how long we've been at top speed
    float waterLevel = 0.0f;     // 0 = empty drum, 1 = full
    float pinkAmount = 0.0f;     // 0 = white clothes are still white, 1 = fully pink

    // --- the clothes ---
    Cloth clothes[CLOTH_COUNT];

    // --- dragging ---
    int dragged = -1;              // which cloth is being dragged (-1 = none)
    float grabDX = 0, grabDY = 0;  // where on the cloth we grabbed it (so it doesn't jump)
    bool wasMouseDown = false;     // was the left button down LAST frame?

    GLuint arrowTex = 0;           // the < > button image
};

// Sets up the layout and the clothes on the left side of the screen
void initScene(Scene& s, float screenW, float screenH,
               GLuint tshirtTex, GLuint pantsTex, GLuint dressTex, GLuint sockTex, GLuint arrowTex);

// Mouse: clicking < > buttons, dragging clothes, dropping them into the drum
void handleMouse(Scene& s, float mouseX, float mouseY, bool mouseDown);

// ENTER was pressed: start the wash (if at least one cloth is in the drum)
void handleEnter(Scene& s);

// Moves everything forward by dt seconds: door, spinning, clothes, water, pink...
void updateScene(Scene& s, float dt);

// Draws the machine, the clothes and the buttons
void drawScene(const Scene& s, float mouseX, float mouseY);
