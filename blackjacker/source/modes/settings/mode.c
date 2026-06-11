#include "mode.h"

#include "graphics/graphics.h"

enum {
    SETTINGS_CONTROL_COUNT = 5,
    SETTINGS_OPTION_COUNT = 4,
    SETTINGS_WIDTH = 44,
};

static const char *soft17Choices[] = {
    "Hit",
    "Stand",
};

static void backToTrain(Runtime_Game *game) {
    (void)Runtime_updateSettings(game, game->playSettings);
    Runtime_queueState(&game->state, MODE_PLAY);
}

static Graphics_Box settingsOptionsBox(void) {
    const Graphics_Box content = Graphics_minimumContentBox();

    return Graphics_positionWithin(
        content,
        (Graphics_Size){SETTINGS_WIDTH, SETTINGS_OPTION_COUNT},
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_CENTER
    );
}

static Graphics_Box settingsBackBox(void) {
    const Graphics_Box content = Graphics_minimumContentBox();

    return Graphics_positionWithin(
        content,
        (Graphics_Size){content.width, 1},
        GRAPHICS_ALIGN_START,
        GRAPHICS_ALIGN_END
    );
}

static Graphics_ControlGroup settingsControls(
    Runtime_Game *game,
    Graphics_Box optionsBox,
    Graphics_Box backBox
) {
    PlaySettings *settings = &game->playSettings;
    static Graphics_Control controls[SETTINGS_CONTROL_COUNT];

    controls[0] = (Graphics_Control){
        .label = "Soft 17",
        .labelWidth = 20,
        .type = GRAPHICS_CONTROL_ENUM_INPUT,
        .data.enumeration =
            {
                (int *)&settings->dealerSoft17Rule,
                soft17Choices,
                2,
            },
    };
    controls[1] = (Graphics_Control){
        .label = "Decks",
        .labelWidth = 20,
        .type = GRAPHICS_CONTROL_INT_INPUT,
        .data.integer = {&settings->deckCount, 1, 8, 1},
    };
    controls[2] = (Graphics_Control){
        .label = "Surrender",
        .labelWidth = 20,
        .type = GRAPHICS_CONTROL_BOOL_INPUT,
        .data.boolean = {&settings->allowSurrender},
    };
    controls[3] = (Graphics_Control){
        .label = "Double After Split",
        .labelWidth = 20,
        .type = GRAPHICS_CONTROL_BOOL_INPUT,
        .data.boolean = {&settings->allowDoubleAfterSplit},
    };
    controls[4] = (Graphics_Control){
        .label = "BACK",
        .labelWidth = 0,
        .labelAlignment = GRAPHICS_ALIGN_START,
        .width = 12,
        .shortcut = 'b',
        .type = GRAPHICS_CONTROL_BUTTON,
        .data.button = {backToTrain},
    };

    Graphics_arrangeControlsWithin(
        optionsBox,
        GRAPHICS_DIRECTION_COLUMN,
        0,
        controls,
        SETTINGS_OPTION_COUNT
    );
    controls[4].tabIndex = 4;
    Graphics_positionControlWithin(
        &controls[4],
        backBox,
        GRAPHICS_ALIGN_END,
        GRAPHICS_ALIGN_STRETCH
    );

    const Graphics_ControlGroup group = {
        controls,
        SETTINGS_CONTROL_COUNT,
        &game->state.modeFocus[MODE_SETTINGS],
    };

    return group;
}

void settingsCallback(Runtime_Game *game, Runtime_ModeEvent event) {
    Graphics_ControlGroup controls =
        settingsControls(game, settingsOptionsBox(), settingsBackBox());

    if(event.type == MODE_EVENT_INPUT) {
        Graphics_handleControlInput(&controls, event.input, game);
        return;
    }

    Graphics_drawControls(controls);
}
