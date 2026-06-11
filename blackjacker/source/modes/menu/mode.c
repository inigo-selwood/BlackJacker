#include "mode.h"

#include "graphics/graphics.h"
#include "runtime/runtime.h"

enum {
    MENU_OPTION_COUNT = 2,
    MENU_OPTION_GAP = 1,
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

static void quitGame(Runtime_Game *game) {
    game->state.running = false;
}

static Graphics_ControlGroup
menuControls(Runtime_Game *game, Graphics_Box box) {
    static Graphics_Control controls[] = {
        {
            .label = "TRAIN",
            .labelWidth = 0,
            .labelAlignment = GRAPHICS_ALIGN_START,
            .width = MENU_OPTION_WIDTH,
            .shortcut = 't',
            .type = GRAPHICS_CONTROL_BUTTON,
            .data.button = {startPlay},
        },
        {
            .label = "QUIT",
            .labelWidth = 0,
            .labelAlignment = GRAPHICS_ALIGN_START,
            .width = MENU_OPTION_WIDTH,
            .shortcut = 'q',
            .type = GRAPHICS_CONTROL_BUTTON,
            .data.button = {quitGame},
        },
    };

    Graphics_positionControlWithin(
        &controls[0],
        box,
        GRAPHICS_ALIGN_START,
        GRAPHICS_ALIGN_STRETCH
    );
    Graphics_positionControlWithin(
        &controls[1],
        box,
        GRAPHICS_ALIGN_END,
        GRAPHICS_ALIGN_STRETCH
    );

    const Graphics_ControlGroup group = {
        controls,
        MENU_OPTION_COUNT,
        &game->state.modeFocus[MODE_MENU],
    };

    return group;
}

void mainMenuCallback(Runtime_Game *game, Runtime_ModeEvent event) {
    const Graphics_Box content = Graphics_minimumContentBox();
    const Graphics_Size bannerSize = Graphics_blockTextSize(&bannerText);
    const int bannerHeight = bannerSize.height > 0 ? bannerSize.height : 1;
    int panelWidth = bannerSize.width > 0 ? bannerSize.width : 32;

    if(panelWidth > content.width) {
        panelWidth = content.width;
    }

    const Graphics_Box bannerBox = Graphics_positionWithin(
        content,
        (Graphics_Size){panelWidth, bannerHeight},
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_CENTER
    );
    const Graphics_Box controlsBox = Graphics_positionWithin(
        content,
        (Graphics_Size){content.width, 1},
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_END
    );
    Graphics_positionBlockTextWithin(
        &bannerText,
        bannerBox,
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_START
    );
    Graphics_ControlGroup controls = menuControls(game, controlsBox);

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
