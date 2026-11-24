#include "counter.h"

void Counter_Update(Counter *counter, bool pressed) {
    if (pressed && !counter->was_pressed) {
        ++counter->value;
    }
    counter->was_pressed = pressed;
}
