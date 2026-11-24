#ifndef COUNTER_H
#define COUNTER_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t value;
    bool was_pressed;
} Counter;

// pressed is true only while the pointer is down over the counter button.
void Counter_Update(Counter *counter, bool pressed);

#endif
