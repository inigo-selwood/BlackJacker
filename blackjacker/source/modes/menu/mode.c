#include "mode.h"

#include "graphics/graphics.h"
#include "runtime/runtime.h"

enum {
    MENU_OPTION_COUNT = 3,
    MENU_OPTION_GAP = 2,
    MENU_OPTION_WIDTH = 14,
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
            .label = "TRAIN",
            .labelWidth = 0,
            .labelAlignment = GRAPHICS_ALIGN_CENTER,
            .width = MENU_OPTION_WIDTH,
            .shortcut = 't',
            .type = GRAPHICS_CONTROL_BUTTON,
            .data.button = {startPlay},
        },
        {
            .label = "SETTINGS",
            .labelWidth = 0,
            .labelAlignment = GRAPHICS_ALIGN_CENTER,
            .width = MENU_OPTION_WIDTH,
            .shortcut = 's',
            .type = GRAPHICS_CONTROL_BUTTON,
            .data.button = {openSettings},
        },
        {
            .label = "QUIT",
            .labelWidth = 0,
            .labelAlignment = GRAPHICS_ALIGN_CENTER,
            .width = MENU_OPTION_WIDTH,
            .shortcut = 'q',
            .type = GRAPHICS_CONTROL_BUTTON,
            .data.button = {quitGame},
        },
    };

    const Graphics_Box row = Graphics_positionWithin(
        box,
        (Graphics_Size){
            MENU_OPTION_COUNT * MENU_OPTION_WIDTH
                + (MENU_OPTION_COUNT - 1) * MENU_OPTION_GAP,
            1,
        },
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_CENTER
    );

    Graphics_arrangeControlsWithin(
        row,
        GRAPHICS_DIRECTION_ROW,
        MENU_OPTION_GAP,
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
        (Graphics_Size){panelWidth, bannerHeight + 5},
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_CENTER
    );
    Graphics_Box sections[3] = {
        {0, 0, 0, bannerHeight},
        {0, 0, 0, 2},
        {0, 0, 0, 1},
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
