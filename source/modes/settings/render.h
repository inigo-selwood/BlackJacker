#ifndef BLACKJACKER_MODES_SETTINGS_RENDER_H
#define BLACKJACKER_MODES_SETTINGS_RENDER_H

#include "graphics/graphics.h"
#include "runtime/runtime.h"

/** Important rectangles used by settings input and rendering. */
typedef struct {
    Graphics_Box title;
    Graphics_Box help;
    Graphics_Box divider;
    Graphics_Box options;
    Graphics_Box back;
} SettingsLayout;

/** Computes settings screen layout rectangles. */
SettingsLayout settingsLayout(void);

/** Draws the settings screen with current control focus context. */
void drawSettings(Runtime_Game *game);

#endif
