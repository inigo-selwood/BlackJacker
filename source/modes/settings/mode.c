#include "mode.h"

#include "graphics/graphics.h"
#include "modes/settings/controls.h"
#include "modes/settings/render.h"

void settingsCallback(Runtime_Game *game, Runtime_ModeEvent event) {
    const SettingsLayout layout = settingsLayout();
    Graphics_ControlGroup controls =
        settingsControls(game, layout.options, layout.back);

    if(event.type == MODE_EVENT_INPUT) {
        Graphics_handleControlInput(&controls, event.input, game);
        return;
    }

    drawSettings(game);
}
