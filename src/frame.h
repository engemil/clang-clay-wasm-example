#ifndef FRAME_H
#define FRAME_H

#include <stdbool.h>
#include <stdint.h>

// scratch receives the render command array's capacity, length, and address.
void UpdateDrawFrame(uintptr_t scratch[3], float width, float height, float mouseWheelX,
                     float mouseWheelY, float mousePositionX, float mousePositionY,
                     bool isTouchDown, bool isMouseDown, float deltaTime);

#endif
