#ifndef BLACKJACKER_MODES_PLAY_CONTROLS_H
#define BLACKJACKER_MODES_PLAY_CONTROLS_H

#include "graphics/graphics.h"
#include "runtime/runtime.h"

/** Builds and positions controls for the current play state. */
Graphics_ControlGroup playControls(Runtime_Game *game, Graphics_Box box);

#endif
