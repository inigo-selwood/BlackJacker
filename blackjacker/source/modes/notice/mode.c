#include "mode.h"

#include <curses.h>

#include "graphics/graphics.h"
#include "runtime/runtime.h"

void noticeCallback(Runtime_Game *game, Runtime_ModeEvent event) {
    (void)game;

    if(event.type != MODE_EVENT_RENDER) {
        return;
    }

    const Graphics_Box screen = Graphics_screenBox();
    const Graphics_Box panel = Graphics_positionWithin(
        screen,
        (Graphics_Size){56, 7},
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_CENTER
    );
    Graphics_Box sections[5] = {
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 1},
    };
    Graphics_Label title = Graphics_label(
        "Console Too Small",
        GRAPHICS_ALIGN_CENTER,
        true,
        false
    );
    Graphics_Label body = Graphics_label(
        "Please enlarge the terminal to continue.",
        GRAPHICS_ALIGN_CENTER,
        false,
        false
    );

    Graphics_arrangeWithin(panel, GRAPHICS_DIRECTION_COLUMN, 0, sections, 5);

    Graphics_positionLabelWithin(
        &title,
        sections[0],
        GRAPHICS_ALIGN_STRETCH,
        GRAPHICS_ALIGN_STRETCH
    );
    Graphics_positionLabelWithin(
        &body,
        sections[2],
        GRAPHICS_ALIGN_STRETCH,
        GRAPHICS_ALIGN_STRETCH
    );
    Graphics_drawLabel(title);
    Graphics_drawLabel(body);
    mvprintw(
        sections[4].y,
        sections[4].x + (sections[4].width - 34) / 2,
        "Minimum: %dx%d  Current: %dx%d",
        MIN_TERMINAL_WIDTH,
        MIN_TERMINAL_HEIGHT,
        screen.width,
        screen.height
    );
}
