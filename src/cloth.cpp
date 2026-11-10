#include "cloth.h"
#include "draw.h"

// --- the < > buttons next to each cloth (switch it between red and white) ---
// (they sit next to the cloth's HOME position, so they don't move when the cloth is dragged)
static const float ARROW_SIZE = 40;   // the round button's size, in pixels
static float leftArrowX(Cloth c)  { return c.homeX - c.width / 2 - 25; }   // center of the < button
static float rightArrowX(Cloth c) { return c.homeX + c.width / 2 + 25; }   // center of the > button

bool isInsideBox(float px, float py, float x, float y, float width, float height) {
    return px > x - width / 2 && px < x + width / 2 &&
           py > y - height / 2 && py < y + height / 2;
}

bool isInside(float px, float py, Cloth c) {
    return isInsideBox(px, py, c.x, c.y, c.width, c.height);
}

bool isOnArrow(float px, float py, Cloth c) {
    return isInsideBox(px, py, leftArrowX(c),  c.homeY, ARROW_SIZE, ARROW_SIZE) ||
           isInsideBox(px, py, rightArrowX(c), c.homeY, ARROW_SIZE, ARROW_SIZE);
}

void drawCloth(Cloth c, float pink) {
    setRotation(c.rotation);   // turn the cloth (tumbling)

    if (c.isRed) {
        drawShape(c.x, c.y, c.width, c.height, 0.85f, 0.10f, 0.10f, 1.0f, false, c.texture);
    } else {
        // white = (1, 1, 1), pink = (1, 0.6, 0.75): blend between them (lerp, like the door)
        float r = 1.0f;
        float g = 1.0f + (0.60f - 1.0f) * pink;
        float b = 1.0f + (0.75f - 1.0f) * pink;
        drawShape(c.x, c.y, c.width, c.height, r, g, b, 1.0f, false, c.texture);
    }

    setRotation(0.0f);         // back to "not rotated" for the next shapes
}

void drawArrows(Cloth c, GLuint arrowTex, float mouseX, float mouseY) {
    float xs[2] = { leftArrowX(c), rightArrowX(c) };
    for (int side = 0; side < 2; side++) {
        bool hover = isInsideBox(mouseX, mouseY, xs[side], c.homeY, ARROW_SIZE, ARROW_SIZE);

        // dark round button behind the arrow (darker when hovered)
        drawShape(xs[side], c.homeY, ARROW_SIZE, ARROW_SIZE, 0.0f, 0.0f, 0.0f, hover ? 0.6f : 0.3f, true);

        // the arrow: arrow.png is "<". For ">" we draw it with a NEGATIVE width, which mirrors it
        float iconWidth = (side == 0) ? 26.0f : -26.0f;
        drawShape(xs[side], c.homeY, iconWidth, 26.0f, 1.0f, 1.0f, 1.0f, hover ? 1.0f : 0.7f, false, arrowTex);
    }
}
