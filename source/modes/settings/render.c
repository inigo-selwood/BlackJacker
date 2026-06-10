#include "render.h"

#include "graphics/graphics.h"
#include "modes/settings/controls.h"

SettingsLayout settingsLayout(void) {
    const Graphics_Box screen = Graphics_screenBox();
    const Graphics_Box panel = Graphics_positionWithin(
        screen,
        (Graphics_Size){44, 22},
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_CENTER
    );
    Graphics_Box sections[] = {
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 3},
        {0, 0, 0, 1},
        {0, 0, 0, 14},
        {0, 0, 0, 1},
        {0, 0, 0, 1},
    };

    Graphics_arrangeWithin(panel, GRAPHICS_DIRECTION_COLUMN, 0, sections, 7);

    const SettingsLayout settings = {
        .title = sections[0],
        .help = sections[2],
        .divider = sections[3],
        .options = sections[4],
        .back = sections[6],
    };

    return settings;
}

void drawSettings(Runtime_Game *game) {
    const SettingsLayout layout = settingsLayout();
    Graphics_ControlGroup controls =
        settingsControls(game, layout.options, layout.back);
    Graphics_Control *focused = Graphics_focusedControl(&controls);
    Graphics_Label title =
        Graphics_label("PLAY SETTINGS", GRAPHICS_ALIGN_CENTER, true, false);
    Graphics_Label help = Graphics_label(
        focused && focused->info ? focused->info : "",
        GRAPHICS_ALIGN_START,
        false,
        false
    );

    Graphics_positionLabelWithin(
        &title,
        layout.title,
        GRAPHICS_ALIGN_STRETCH,
        GRAPHICS_ALIGN_STRETCH
    );
    Graphics_positionLabelWithin(
        &help,
        layout.help,
        GRAPHICS_ALIGN_STRETCH,
        GRAPHICS_ALIGN_STRETCH
    );

    Graphics_drawLabel(title);
    Graphics_drawWrappedLabel(help);
    Graphics_drawHorizontalDivider(layout.divider);
    Graphics_drawControls(controls);
}
