#include "mode.h"

#include "graphics/graphics.h"
#include "modes/play/controls.h"
#include "modes/play/render.h"

static Graphics_Box controlsBox(void) {
    const Graphics_Box screen = Graphics_screenBox();
    const Graphics_Box panel = Graphics_positionWithin(
        screen,
        (Graphics_Size){78, 22},
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_CENTER
    );
    Graphics_Box sections[9] = {
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 7},
        {0, 0, 0, 1},
        {0, 0, 0, 7},
        {0, 0, 0, 2},
        {0, 0, 0, 1},
    };

    Graphics_arrangeWithin(panel, GRAPHICS_DIRECTION_COLUMN, 0, sections, 9);

    return sections[8];
}

void playCallback(Runtime_Game *game, Runtime_ModeEvent event) {
    Graphics_ControlGroup controls = playControls(game, controlsBox());

    if(event.type == MODE_EVENT_INPUT) {
        Graphics_handleControlInput(&controls, event.input, game);
        return;
    }

    drawPlay(game);
}
