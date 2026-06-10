#include "mode.h"

#include "graphics/graphics.h"
#include "runtime/runtime.h"

enum {
    MENU_OPTION_COUNT = 3,
};

static Graphics_BlockText bannerText = {
    .path = "resources/banner.txt",
    .fallbackPath = "../resources/banner.txt",
    .fallbackText = "BlackJacker",
    .alignment = GRAPHICS_ALIGN_CENTER,
    .bold = true,
};

static void startPlay(Runtime_Game *game) {
    Runtime_queueState(&game->state, MODE_PLAY);
}

static void openSettings(Runtime_Game *game) {
    Runtime_queueState(&game->state, MODE_SETTINGS);
}

static void quitGame(Runtime_Game *game) {
    game->state.running = false;
}

static Graphics_ControlGroup
menuControls(Runtime_Game *game, Graphics_Box box) {
    static Graphics_Control controls[] = {
        {
            .label = "PLAY",
            .labelWidth = 0,
            .labelAlignment = GRAPHICS_ALIGN_CENTER,
            .type = GRAPHICS_CONTROL_BUTTON,
            .data.button = {startPlay},
        },
        {
            .label = "SETTINGS",
            .labelWidth = 0,
            .labelAlignment = GRAPHICS_ALIGN_CENTER,
            .type = GRAPHICS_CONTROL_BUTTON,
            .data.button = {openSettings},
        },
        {
            .label = "QUIT",
            .labelWidth = 0,
            .labelAlignment = GRAPHICS_ALIGN_CENTER,
            .type = GRAPHICS_CONTROL_BUTTON,
            .data.button = {quitGame},
        },
    };

    Graphics_arrangeControlsWithin(
        box,
        GRAPHICS_DIRECTION_COLUMN,
        1,
        controls,
        MENU_OPTION_COUNT
    );

    const Graphics_ControlGroup group = {
        controls,
        MENU_OPTION_COUNT,
        &game->state.modeFocus[MODE_MENU],
    };

    return group;
}

void mainMenuCallback(Runtime_Game *game, Runtime_ModeEvent event) {
    const Graphics_Box screen = Graphics_screenBox();
    const Graphics_Size bannerSize = Graphics_blockTextSize(&bannerText);
    const int bannerHeight = bannerSize.height > 0 ? bannerSize.height : 1;
    int panelWidth = bannerSize.width > 0 ? bannerSize.width : 32;

    if(panelWidth > screen.width) {
        panelWidth = screen.width;
    }

    const Graphics_Box panel = Graphics_positionWithin(
        screen,
        (Graphics_Size){panelWidth, bannerHeight + 7},
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_CENTER
    );
    Graphics_Box sections[3] = {
        {0, 0, 0, bannerHeight},
        {0, 0, 0, 2},
        {0, 0, 0, MENU_OPTION_COUNT * 2 - 1},
    };
    Graphics_arrangeWithin(panel, GRAPHICS_DIRECTION_COLUMN, 0, sections, 3);
    Graphics_positionBlockTextWithin(
        &bannerText,
        sections[0],
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_START
    );
    Graphics_ControlGroup controls = menuControls(game, sections[2]);

    if(event.type == MODE_EVENT_RENDER) {
        Graphics_drawBlockText(&bannerText);
        Graphics_drawControls(controls);
        return;
    }

    if(event.type == MODE_EVENT_INPUT
        && Graphics_handleControlInput(&controls, event.input, game)) {
        return;
    }

    (void)event;
}
