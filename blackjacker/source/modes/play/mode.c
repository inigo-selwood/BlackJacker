#include "mode.h"

#include "graphics/graphics.h"
#include "modes/play/controls.h"
#include "modes/play/render.h"

static Graphics_Box controlsBox(void) {
    const Graphics_Box content = Graphics_minimumContentBox();

    return Graphics_positionWithin(
        content,
        (Graphics_Size){content.width, 4},
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_END
    );
}

void playCallback(Runtime_Game *game, Runtime_ModeEvent event) {
    Graphics_ControlGroup controls = playControls(game, controlsBox());

    if(event.type == MODE_EVENT_INPUT) {
        Graphics_handleControlInput(&controls, event.input, game);
        return;
    }

    drawPlay(game);
    Graphics_drawHorizontalDivider((Graphics_Box){
        controlsBox().x,
        controlsBox().y + 2,
        controlsBox().width,
        1,
    });
    Graphics_drawControls(controls);
}
