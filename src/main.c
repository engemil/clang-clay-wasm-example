// pure_c_example_project — web UI in C with Clay (submodule at src/clay, pinned to v0.14).
//
// Built freestanding: clang --target=wasm32 -nostdlib. There is no libc, so the
// click counter formats its own decimal string.
//
// Browser-side contract with web/index.html (Clay's reference HTML renderer):
//   - JS allocates Clay's arena inside wasm linear memory at __heap_base + 1024
//     and calls Clay_Initialize / Clay_CreateArenaWithCapacityAndMemory itself.
//   - Every frame JS calls UpdateDrawFrame(scratch, width, height, wheelX, wheelY,
//     mouseX, mouseY, touchDown, mouseDown, deltaTime).
//   - We must leave {capacity, length, internalArray} as 3 x uint32 at scratch,
//     which JS reads to walk the Clay_RenderCommandArray and build DOM nodes.

#define CLAY_IMPLEMENTATION
#include "clay.h"

#include <stdbool.h>
#include <stdint.h>

static const Clay_Color COLOR_TEXT        = {30, 30, 30, 255};
static const Clay_Color COLOR_BUTTON      = {50, 50, 50, 255};
static const Clay_Color COLOR_BUTTON_TEXT = {255, 255, 255, 255};

static uint32_t clickCount = 0;

// --- tiny u32 -> decimal formatter (no sprintf in -nostdlib wasm) -----------
static Clay_String U32ToString(uint32_t value, char buffer[12]) {
    uint32_t pos = 11;
    if (value == 0) {
        buffer[--pos] = '0';
    }
    while (value > 0) {
        buffer[--pos] = (char)('0' + (value % 10));
        value /= 10;
    }
    return (Clay_String) {
        .isStaticallyAllocated = false,
        .length = 11 - pos,
        .chars = &buffer[pos],
    };
}

CLAY_WASM_EXPORT("UpdateDrawFrame")
void UpdateDrawFrame(
    uint32_t scratchAddress, float width, float height,
    float mouseWheelX, float mouseWheelY, float mousePositionX, float mousePositionY,
    bool isTouchDown, bool isMouseDown, float deltaTime)
{
    Clay_SetLayoutDimensions((Clay_Dimensions) {width, height});
    Clay_SetPointerState((Clay_Vector2) {mousePositionX, mousePositionY}, isMouseDown || isTouchDown);
    Clay_UpdateScrollContainers(true, (Clay_Vector2) {mouseWheelX, mouseWheelY}, deltaTime);

    // Click counter: edge-triggered so holding the button counts once.
    // (isMouseDown is only true for a single frame, but isTouchDown persists.)
    static bool counterDownPrevious = false;
    bool counterDown = (isMouseDown || isTouchDown) && Clay_PointerOver(CLAY_ID("CounterButton"));
    if (counterDown && !counterDownPrevious) {
        clickCount++;
    }
    counterDownPrevious = counterDown;

    // The browser reads this text from the render commands after this call returns.
    static char countBuffer[12];
    Clay_String countText = U32ToString(clickCount, countBuffer);

    Clay_BeginLayout();
    CLAY({ .id = CLAY_ID("Page"),
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
            .padding = CLAY_PADDING_ALL(24),
            .childGap = 12,
        },
    }) {
        CLAY_TEXT(CLAY_STRING("Example Project"),
            CLAY_TEXT_CONFIG({ .fontSize = 36, .textColor = COLOR_TEXT }));
        CLAY_TEXT(CLAY_STRING("\"Click me\" counter"),
            CLAY_TEXT_CONFIG({ .fontSize = 24, .textColor = COLOR_TEXT }));
        CLAY_TEXT(CLAY_STRING("Running C code with WebAssembly (wasm) and Clay library in your browser."),
            CLAY_TEXT_CONFIG({ .fontSize = 20, .textColor = COLOR_TEXT }));
        CLAY_TEXT(countText, CLAY_TEXT_CONFIG({ .fontSize = 36, .textColor = COLOR_TEXT }));

        CLAY({ .id = CLAY_ID("CounterButton"),
            .layout = {
                .sizing = {CLAY_SIZING_FIT(0), CLAY_SIZING_FIXED(44)},
                .padding = {.left = 16, .right = 16},
                .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
            },
            .backgroundColor = COLOR_BUTTON,
            .cornerRadius = CLAY_CORNER_RADIUS(4),
        }) {
            CLAY_TEXT(CLAY_STRING("Click me"),
                CLAY_TEXT_CONFIG({ .fontSize = 20, .textColor = COLOR_BUTTON_TEXT }));
        }
    }

    // Hand the render command array to JS via the scratch address.
    Clay_RenderCommandArray commands = Clay_EndLayout();
    uint32_t *scratch = (uint32_t *)(uintptr_t)scratchAddress;
    scratch[0] = commands.capacity;
    scratch[1] = commands.length;
    scratch[2] = (uint32_t)(uintptr_t)commands.internalArray;
}
