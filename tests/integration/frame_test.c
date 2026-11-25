#include "clay.h"
#include "frame.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Clay_Dimensions MeasureText(Clay_StringSlice text, Clay_TextElementConfig *config,
                                   void *userData) {
    (void)userData;
    float width = (float)text.length * (float)config->fontSize / 2.0f;
    return (Clay_Dimensions){.width = width, .height = config->fontSize};
}

static void FailOnClayError(Clay_ErrorData error) {
    fprintf(stderr, "%.*s\n", error.errorText.length, error.errorText.chars);
    abort();
}

static void ExpectRenderedCount(float x, float y, bool touchDown, bool mouseDown,
                                const char *expected) {
    uintptr_t scratch[3];
    UpdateDrawFrame(scratch, 800, 600, 0, 0, x, y, touchDown, mouseDown, 1.0f / 60.0f);
    const Clay_RenderCommand *commands = (const Clay_RenderCommand *)scratch[2];
    for (uintptr_t i = 0; i < scratch[1]; ++i) {
        if (commands[i].commandType != CLAY_RENDER_COMMAND_TYPE_TEXT) {
            continue;
        }
        Clay_StringSlice text = commands[i].renderData.text.stringContents;
        if ((size_t)text.length == strlen(expected) &&
            memcmp(text.chars, expected, text.length) == 0) {
            return;
        }
    }
    fprintf(stderr, "Expected the rendered counter to show %s\n", expected);
    abort();
}

void TestFrame(void) {
    size_t size = Clay_MinMemorySize();
    void *memory = malloc(size);
    assert(memory);
    Clay_Initialize(Clay_CreateArenaWithCapacityAndMemory(size, memory),
                    (Clay_Dimensions){.width = 800, .height = 600},
                    (Clay_ErrorHandler){.errorHandlerFunction = FailOnClayError});
    Clay_SetMeasureTextFunction(MeasureText, NULL);

    ExpectRenderedCount(0, 0, false, true, "0");
    Clay_ElementData button = Clay_GetElementData(CLAY_ID("CounterButton"));
    assert(button.found);
    float x = button.boundingBox.x + button.boundingBox.width / 2;
    float y = button.boundingBox.y + button.boundingBox.height / 2;
    ExpectRenderedCount(x, y, true, false, "1");
    ExpectRenderedCount(x, y, true, false, "1");
    for (int count = 2; count <= 10; ++count) {
        char expected[12];
        snprintf(expected, sizeof expected, "%d", count - 1);
        ExpectRenderedCount(x, y, false, false, expected);
        snprintf(expected, sizeof expected, "%d", count);
        ExpectRenderedCount(x, y, false, true, expected);
    }
    Clay_SetCurrentContext(NULL);
    free(memory);
}
