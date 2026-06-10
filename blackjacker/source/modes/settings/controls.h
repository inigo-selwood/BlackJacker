#ifndef BLACKJACKER_MODES_SETTINGS_CONTROLS_H
#define BLACKJACKER_MODES_SETTINGS_CONTROLS_H

#include "graphics/graphics.h"
#include "runtime/runtime.h"

/** Builds and positions controls for the play settings screen. */
Graphics_ControlGroup settingsControls(
    Runtime_Game *game,
    Graphics_Box optionsBox,
    Graphics_Box backBox
);

#endif
