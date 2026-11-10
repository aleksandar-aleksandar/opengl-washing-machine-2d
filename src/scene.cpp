#include "scene.h"
#include "draw.h"

#include <cmath>   // sqrt, sin, cos, atan2, ceil, fabs

// ===================================================================== settings
// (change these to tweak how the machine behaves)

static const float PI = 3.14159265f;

static const float doorCloseDuration  = 1.0f;     // the door takes 1 second to close (and to open)
static const float maxSpinSpeed       = 3.0f;     // top speed (3 rad/s = about half a turn per second)
static const float spinAcceleration   = 1.0f;     // how much faster per second (so 3 seconds to reach top speed)
static const float spinDuration       = 20.0f;    // spin at top speed for 20 seconds
static const float gravity            = 1500.0f;  // how fast falling speeds up, in pixels per second, per second
static const float waterDrainDuration = 2.0f;     // after the wash, the water drains out in 2 seconds
static const float pinkDuration       = 20.0f;    // how many seconds of washing (with 1 red) until fully pink

// ===================================================================== setup

void initScene(Scene& s, float screenW, float screenH,
               GLuint tshirtTex, GLuint pantsTex, GLuint dressTex, GLuint sockTex, GLuint arrowTex) {
    s.screenW = screenW;
    s.screenH = screenH;

    // --- layout (all in pixels) ---
    s.machineX = screenW / 2;               // center of the machine body
    s.machineY = screenH / 2;
    s.drumX = s.machineX;                   // center of the drum
    s.drumY = s.machineY - 50;
    s.drumRadius = 210;
    s.pathRadius = s.drumRadius - 70;       // 70 = half a cloth, so they stay inside the drum
    s.doorRadius = 230;                     // the door frame's radius (a bit bigger than the drum)
    s.glassRadius = 190;                    // the glass inside the frame
    s.hingeX = s.drumX + s.doorRadius;      // the door turns around a hinge on the drum's right side
    s.doorX = s.hingeX + s.doorRadius;      // starts OPEN: to the right of the hinge

    s.arrowTex = arrowTex;

    // the clothes, stacked on the left side of the screen: 3 white + 1 red
    //                x    y                  width  height  texture    isRed  inMachine
    s.clothes[0] = { 200, screenH / 2 + 270,  140,   140,    tshirtTex, false, false };
    s.clothes[1] = { 200, screenH / 2 +  90,  140,   140,    pantsTex,  false, false };
    s.clothes[2] = { 200, screenH / 2 -  90,  140,   140,    dressTex,  false, false };
    s.clothes[3] = { 200, screenH / 2 - 270,  140,   140,    sockTex,   true,  false };   // the red sock!

    for (int i = 0; i < CLOTH_COUNT; i++) {   // remember where each one started
        s.clothes[i].homeX = s.clothes[i].x;
        s.clothes[i].homeY = s.clothes[i].y;
    }
}

// ===================================================================== input

void handleMouse(Scene& s, float mouseX, float mouseY, bool mouseDown) {
    // 1. button JUST pressed (down now, up last frame)
    //    (only while loading: once the wash starts, nothing can be clicked or dragged)
    if (s.state == LOADING && mouseDown && !s.wasMouseDown) {
        // first: was it a click on a < > button? then switch that cloth red <-> white
        bool clickedArrow = false;
        for (int i = 0; i < CLOTH_COUNT; i++) {
            Cloth& c = s.clothes[i];
            if (!c.inMachine && isOnArrow(mouseX, mouseY, c)) {
                c.isRed = !c.isRed;   // ! = "not": true becomes false, false becomes true
                clickedArrow = true;
                break;
            }
        }

        // otherwise: grab the cloth under the mouse
        if (!clickedArrow) {
            for (int i = CLOTH_COUNT - 1; i >= 0; i--) {   // backwards = the one drawn on top first
                Cloth& c = s.clothes[i];
                if (!c.inMachine && isInside(mouseX, mouseY, c)) {
                    s.dragged = i;
                    s.grabDX = c.x - mouseX;
                    s.grabDY = c.y - mouseY;
                    break;   // only grab one
                }
            }
        }
    }

    // 2. button held: the grabbed cloth follows the mouse
    if (mouseDown && s.dragged != -1) {
        s.clothes[s.dragged].x = mouseX + s.grabDX;
        s.clothes[s.dragged].y = mouseY + s.grabDY;
    }

    // 3. button released: drop it, and check if it landed in the drum
    if (!mouseDown && s.dragged != -1) {
        Cloth& c = s.clothes[s.dragged];
        float dx = c.x - s.drumX;
        float dy = c.y - s.drumY;
        float dist = std::sqrt(dx * dx + dy * dy);   // distance from the drum's center

        if (dist < s.drumRadius) {
            c.inMachine = true;
            // if it's too close to the edge, pull it inward so it sits fully inside the drum
            float maxDist = s.drumRadius - 80;
            if (dist > maxDist) {
                c.x = s.drumX + dx / dist * maxDist;
                c.y = s.drumY + dy / dist * maxDist;
            }
        }
        s.dragged = -1;
    }

    s.wasMouseDown = mouseDown;   // remember for next frame
}

void handleEnter(Scene& s) {
    if (s.state != LOADING) return;

    int loaded = 0;
    for (int i = 0; i < CLOTH_COUNT; i++)
        if (s.clothes[i].inMachine) loaded++;

    if (loaded > 0) {
        s.state = DOOR_CLOSING;
        s.dragged = -1;   // if you were holding a cloth, let go of it (it disappears anyway)
    }
}

// ===================================================================== update (one step per frame)

// --- door closing: progress 0 -> 1 over doorCloseDuration seconds ---
static void updateDoorClosing(Scene& s, float dt) {
    s.doorProgress += dt / doorCloseDuration;   // a bit more closed every frame
    if (s.doorProgress >= 1.0f) {
        s.doorProgress = 1.0f;

        // fully closed: every cloth first drops down to the bottom of the drum (gravity),
        // and from there the circle-then-fall cycle begins
        for (int i = 0; i < CLOTH_COUNT; i++) {
            s.clothes[i].falling = true;
            s.clothes[i].fallSpeed = 0.0f;
        }
        s.spinSpeed = 0.0f;
        s.state = SPIN_UP;
    }
}

// --- spin up: faster every frame until top speed; the water rises with the speed ---
static void updateSpinUp(Scene& s, float dt) {
    s.spinSpeed += spinAcceleration * dt;
    s.waterLevel = s.spinSpeed / maxSpinSpeed;   // full exactly at top speed
    if (s.spinSpeed >= maxSpinSpeed) {
        s.spinSpeed = maxSpinSpeed;
        s.waterLevel = 1.0f;
        s.spinTimer = 0.0f;
        s.state = SPINNING;
    }
}

// --- spinning at top speed: count the 20 seconds ---
static void updateSpinning(Scene& s, float dt) {
    s.spinTimer += dt;
    if (s.spinTimer >= spinDuration) {
        s.spinSpeed = 0.0f;
        s.state = STOPPING;
    }
}

// --- move the clothes: ride the circle up to the top, fall down, repeat ---
static void moveClothes(Scene& s, float dt) {
    for (int i = 0; i < CLOTH_COUNT; i++) {
        Cloth& c = s.clothes[i];
        if (!c.inMachine) continue;

        // tumbling: the cloth turns clockwise along with the drum (while riding AND falling)
        c.rotation -= s.spinSpeed * dt;

        if (c.falling) {
            // FALLING: straight down, faster every frame (x doesn't change)
            c.fallSpeed += gravity * dt;
            c.y -= c.fallSpeed * dt;

            // did it hit the bottom edge? (below the center, and at least pathRadius away from it)
            float dx = c.x - s.drumX;
            float dy = c.y - s.drumY;
            if (dy < 0 && dx * dx + dy * dy >= s.pathRadius * s.pathRadius) {
                c.falling = false;
                c.angle = std::atan2(dy, dx);   // where on the circle it landed
                // atan2 gives -PI..PI; we want the angle ABOVE PI/2 (the top), so we can
                // detect "reached the top" as the angle counting down past PI/2
                if (c.angle <= PI / 2) c.angle += 2 * PI;
            }
        }

        if (!c.falling) {
            // RIDING: move clockwise along the circle (MINUS = clockwise)
            c.angle -= s.spinSpeed * dt;

            // reached the top (PI/2 = straight up)? start falling from there
            if (c.angle <= PI / 2) {
                c.angle = PI / 2;
                c.falling = true;
                c.fallSpeed = 0.0f;
            }

            // angle -> x, y on the circle
            c.x = s.drumX + std::cos(c.angle) * s.pathRadius;
            c.y = s.drumY + std::sin(c.angle) * s.pathRadius;
        }
    }
}

// --- pink: red clothes in the drum make the white ones pinker ---
// the more red clothes, the faster: 1 red = fully pink after 20 s, 2 red = after 10 s, ...
static void updatePink(Scene& s, float dt) {
    int redCount = 0;
    for (int i = 0; i < CLOTH_COUNT; i++)
        if (s.clothes[i].isRed && s.clothes[i].inMachine) redCount++;

    s.pinkAmount += dt / pinkDuration * redCount;   // 0 red = no change
    if (s.pinkAmount > 1.0f) s.pinkAmount = 1.0f;   // never more than fully pink
}

// --- stopping: drain the water; once it's empty AND no cloth is in the air, open the door ---
static void updateStopping(Scene& s, float dt) {
    s.waterLevel -= dt / waterDrainDuration;   // a bit lower every frame
    if (s.waterLevel < 0.0f) s.waterLevel = 0.0f;

    bool anyFalling = false;
    for (int i = 0; i < CLOTH_COUNT; i++)
        if (s.clothes[i].inMachine && s.clothes[i].falling) anyFalling = true;

    if (!anyFalling && s.waterLevel == 0.0f)
        s.state = DOOR_OPENING;
}

// --- door opening: the same as closing, but backwards (progress goes 1 -> 0) ---
static void updateDoorOpening(Scene& s, float dt) {
    s.doorProgress -= dt / doorCloseDuration;
    if (s.doorProgress <= 0.0f) {
        s.doorProgress = 0.0f;
        s.state = DONE;   // nothing more happens, only ESC works
    }
}

// --- door swing: turn doorProgress into where the door is and how wide it looks ---
static void updateDoorSwing(Scene& s) {
    // "easing": instead of a constant speed, start slow, speed up, slow down at the end.
    // smoothstep formula: p * p * (3 - 2p)  ->  0 stays 0, 1 stays 1, but the middle is faster
    float p = s.doorProgress;
    float eased = p * p * (3.0f - 2.0f * p);

    // it turns around the hinge, like a real door seen from the front:
    // progress 0 (open) = turned 180 degrees (PI), progress 1 (closed) = 0 degrees
    float doorAngle = PI * (1.0f - eased);
    float swing = std::cos(doorAngle);              // closed: 1, edge-on: 0, open: -1
    s.doorX = s.hingeX - s.doorRadius * swing;      // closed: over the drum, open: right of the hinge
    s.doorSquash = std::fabs(swing);                // seen from the front, a turned door looks narrower
}

void updateScene(Scene& s, float dt) {
    // each state does its own thing (see the State list in scene.h)
    if (s.state == DOOR_CLOSING) updateDoorClosing(s, dt);

    if (s.state == SPIN_UP)       updateSpinUp(s, dt);
    else if (s.state == SPINNING) updateSpinning(s, dt);

    // the clothes move while spinning, and also in STOPPING (spinSpeed is 0 then,
    // so riding clothes stay still, but falling ones still land)
    if (s.state == SPIN_UP || s.state == SPINNING || s.state == STOPPING) moveClothes(s, dt);

    if (s.state == SPIN_UP || s.state == SPINNING) updatePink(s, dt);
    if (s.state == STOPPING)     updateStopping(s, dt);
    if (s.state == DOOR_OPENING) updateDoorOpening(s, dt);

    updateDoorSwing(s);
}

// ===================================================================== drawing

// --- the machine body, control panel, power LED and the time display ---
static void drawMachineBody(const Scene& s) {
    //        x           y                 width  height  R      G      B      A     circle?
    drawShape(s.machineX, s.machineY,       600,   700,    0.85f, 0.85f, 0.85f, 1.0f, false);  // body
    drawShape(s.machineX, s.machineY + 300, 600,   100,    0.70f, 0.70f, 0.72f, 1.0f, false);  // control panel strip on top

    // --- power LED: green while the machine is running, dim red when off ---
    bool machineOn = (s.state != LOADING && s.state != DONE);
    float ledX = s.machineX + 70;
    float ledY = s.machineY + 300;
    if (machineOn) {
        drawShape(ledX, ledY, 44, 44, 0.2f, 1.0f, 0.3f, 0.25f, true);   // soft glow: a bigger, see-through circle
        drawShape(ledX, ledY, 20, 20, 0.3f, 1.0f, 0.4f, 1.0f, true);    // the LED itself: bright green
    } else {
        drawShape(ledX, ledY, 20, 20, 0.35f, 0.08f, 0.08f, 1.0f, true); // off: dark red, no glow
    }

    // --- the time display ---
    int secondsLeft = -1;                                          // -1 = not started, shows "--"
    if (s.state == SPIN_UP)
        secondsLeft = (int)spinDuration;                           // still speeding up: the full 20
    else if (s.state == SPINNING)
        secondsLeft = (int)std::ceil(spinDuration - s.spinTimer);  // counting down 20, 19, ... 1
    else if (s.state == STOPPING || s.state == DOOR_OPENING || s.state == DONE)
        secondsLeft = 0;

    float displayX = s.machineX + 170;
    float displayY = s.machineY + 300;
    drawShape(displayX, displayY, 130, 70, 0.05f, 0.08f, 0.06f, 1.0f, false);   // dark screen
    if (secondsLeft < 0) {
        drawDigit(displayX - 22, displayY, 46, -1);                 // -
        drawDigit(displayX + 22, displayY, 46, -1);                 // -
    } else {
        drawDigit(displayX - 22, displayY, 46, secondsLeft / 10);   // tens:  17 / 10 = 1
        drawDigit(displayX + 22, displayY, 46, secondsLeft % 10);   // ones:  17 % 10 = 7
    }
}

// --- the drum, the "drop here" glow, the clothes inside and the water ---
static void drawDrum(const Scene& s) {
    drawShape(s.drumX, s.drumY, s.drumRadius * 2, s.drumRadius * 2, 0.25f, 0.25f, 0.28f, 1.0f, true);

    // drop highlight: while a cloth is dragged over the drum, a glowing ring shows "drop here"
    if (s.dragged != -1) {
        float dx = s.clothes[s.dragged].x - s.drumX;
        float dy = s.clothes[s.dragged].y - s.drumY;
        if (dx * dx + dy * dy < s.drumRadius * s.drumRadius) {   // the same check as when dropping
            float glowSize = s.drumRadius * 2 + 24;               // 12 px wider than the drum on each side
            drawRing(s.drumX, s.drumY, glowSize, glowSize, (s.drumRadius * 2) / glowSize,
                     0.4f, 0.9f, 1.0f, 0.7f);                     // light blue, a bit see-through
        }
    }

    // clothes INSIDE the drum (drawn before the door, so the door covers them)
    for (int i = 0; i < CLOTH_COUNT; i++) {
        if (s.clothes[i].inMachine)
            drawCloth(s.clothes[i], s.pinkAmount);
    }

    // water: drawn OVER the clothes (see-through), so they look like they're under water
    if (s.waterLevel > 0.0f)
        drawWater(s.drumX, s.drumY, s.drumRadius * 2, s.waterLevel);
}

// --- the hinge and the door (squashed horizontally while it swings) ---
static void drawDoor(const Scene& s) {
    float doorY = s.drumY;
    drawShape(s.hingeX, doorY, 18, 70, 0.45f, 0.45f, 0.48f, 1.0f, false);   // hinge: a small dark block

    float frameWidth = 2 * s.doorRadius * s.doorSquash;
    if (frameWidth < 12) frameWidth = 12;   // edge-on, the door is still a thin 12px line, not invisible
    float glassWidth = 2 * s.glassRadius * s.doorSquash;

    drawRing(s.doorX, doorY, frameWidth, 2 * s.doorRadius, s.glassRadius / s.doorRadius,
             0.75f, 0.75f, 0.78f, 1.0f);                        // frame (a ring, so the middle is empty)
    drawShape(s.doorX, doorY, glassWidth, 2 * s.glassRadius,
              0.20f, 0.20f, 0.22f, 0.5f, true);                 // glass: dark gray, 50% see-through
}

// --- clothes OUTSIDE the drum + their < > buttons (only while loading) ---
static void drawLooseClothes(const Scene& s, float mouseX, float mouseY) {
    if (s.state != LOADING) return;   // they disappear when the wash starts

    // the < > buttons, at each cloth's home spot (drawn first, so clothes go over them)
    for (int i = 0; i < CLOTH_COUNT; i++) {
        if (!s.clothes[i].inMachine)
            drawArrows(s.clothes[i], s.arrowTex, mouseX, mouseY);
    }
    for (int i = 0; i < CLOTH_COUNT; i++) {
        if (!s.clothes[i].inMachine && i != s.dragged)   // skip the dragged one for now...
            drawCloth(s.clothes[i]);
    }
    if (s.dragged != -1)
        drawCloth(s.clothes[s.dragged]);   // ...and draw it LAST, so it's on top of everything
}

void drawScene(const Scene& s, float mouseX, float mouseY) {
    // order matters: later things are drawn on top of earlier things
    drawMachineBody(s);
    drawDrum(s);
    drawDoor(s);
    drawLooseClothes(s, mouseX, mouseY);
}
