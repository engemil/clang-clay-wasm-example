#include "counter.h"

#include <assert.h>

void TestCounter(void) {
    Counter counter = {0};

    Counter_Update(&counter, false); // No active press over the button.
    assert(counter.value == 0);

    Counter_Update(&counter, true);
    assert(counter.value == 1);

    Counter_Update(&counter, true); // Holding does not count again.
    assert(counter.value == 1);

    Counter_Update(&counter, false);
    assert(counter.value == 1);

    Counter_Update(&counter, true);
    assert(counter.value == 2);
}
