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

static const Clay_Color COLOR_BG       = {43,  41,  51,  255};
static const Clay_Color COLOR_PANEL    = {224, 215, 210, 255};
static const Clay_Color COLOR_ACCENT   = {168, 66,  28,  255};
static const Clay_Color COLOR_ORANGE   = {225, 138, 50,  255};
static const Clay_Color COLOR_TEXT     = {255, 255, 255, 255};
static const Clay_Color COLOR_DARKTEXT = {60,  60,  70,  255};

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

static void SidebarItemComponent(int index) {
    CLAY({ .id = CLAY_IDI("SidebarItem", index), .layout = {.padding = CLAY_PADDING_ALL(16)} }) {
        CLAY_TEXT(CLAY_STRING("Sidebar item"), CLAY_TEXT_CONFIG({ .fontId = 0, .fontSize = 16, .textColor = COLOR_DARKTEXT }));
    }
}

CLAY_WASM_EXPORT("UpdateDrawFrame")
void UpdateDrawFrame(
    uint32_t scratchAddress, float width, float height,
    float mouseWheelX, float mouseWheelY, float mousePositionX, float mousePositionY,
    bool isTouchDown, bool isMouseDown, float deltaTime)
{
    (void)width; (void)height; (void)deltaTime;

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

    Clay_BeginLayout();

    CLAY({ .id = CLAY_ID("OuterContainer"),
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
        },
        .backgroundColor = COLOR_BG,
    }) {
        // Header bar
        CLAY({ .id = CLAY_ID("Header"),
            .layout = {
                .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(64)},
                .padding = CLAY_PADDING_ALL(24),
                .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
            },
            .backgroundColor = COLOR_PANEL,
        }) {
            CLAY_TEXT(CLAY_STRING("pure_c_example_project"), CLAY_TEXT_CONFIG({ .fontId = 0, .fontSize = 24, .textColor = COLOR_DARKTEXT }));
        }

        CLAY({ .id = CLAY_ID("ContentRow"),
            .layout = {
                .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                .padding = CLAY_PADDING_ALL(16),
                .childGap = 16,
            },
        }) {
            // Sidebar
            CLAY({ .id = CLAY_ID("Sidebar"),
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .sizing = {CLAY_SIZING_FIXED(240), CLAY_SIZING_GROW(0)},
                    .padding = CLAY_PADDING_ALL(16),
                    .childGap = 8,
                },
                .backgroundColor = COLOR_PANEL,
            }) {
                for (int i = 0; i < 5; i++) {
                    SidebarItemComponent(i);
                }
            }

            // Main card with the interactive button
            CLAY({ .id = CLAY_ID("MainCard"),
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                    .padding = CLAY_PADDING_ALL(32),
                    .childGap = 24,
                },
                .backgroundColor = COLOR_PANEL,
            }) {
                CLAY_TEXT(CLAY_STRING("Hello from C, running as wasm in your browser."),
                    CLAY_TEXT_CONFIG({ .fontId = 0, .fontSize = 20, .textColor = COLOR_DARKTEXT }));

                char countBuffer[12];
                Clay_String countText = U32ToString(clickCount, countBuffer);
                CLAY_TEXT(countText, CLAY_TEXT_CONFIG({ .fontId = 0, .fontSize = 48, .textColor = COLOR_ACCENT }));

                CLAY({ .id = CLAY_ID("CounterButton"),
                    .layout = {
                        .sizing = {CLAY_SIZING_FIXED(180), CLAY_SIZING_FIXED(56)},
                        .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER},
                    },
                    .backgroundColor = COLOR_ORANGE,
                    .cornerRadius = CLAY_CORNER_RADIUS(8),
                }) {
                    CLAY_TEXT(CLAY_STRING("Click me"), CLAY_TEXT_CONFIG({ .fontId = 0, .fontSize = 18, .textColor = COLOR_TEXT }));
                }
            }
        }
    }

    // Hand the render command array to JS via the scratch address.
    Clay_RenderCommandArray commands = Clay_EndLayout();
    uint32_t *scratch = (uint32_t *)(uintptr_t)scratchAddress;
    scratch[0] = commands.capacity;
    scratch[1] = commands.length;
    scratch[2] = (uint32_t)(uintptr_t)commands.internalArray;
}
