#include "mode.h"

#include "runtime/runtime.h"

enum { CONTROL_GAP = 1, CONTROL_WIDTH = 14 };

static void noop(Runtime_Game *game) {
    (void)game;
}

static void openSettings(Runtime_Game *game) {
    Runtime_queueState(&game->state, MODE_SETTINGS);
}

static void openGuide(Runtime_Game *game) {
    Runtime_queueState(&game->state, MODE_GUIDE);
}

static void quitPlay(Runtime_Game *game) {
    Runtime_queueState(&game->state, MODE_MENU);
}

static void addAction(
    Graphics_Control *controls,
    int *count,
    const char *label,
    int shortcut,
    Graphics_ButtonAction action
) {
    controls[*count] = (Graphics_Control){
        .label = label,
        .labelWidth = 0,
        .labelAlignment = GRAPHICS_ALIGN_START,
        .width = CONTROL_WIDTH,
        .shortcut = shortcut,
        .type = GRAPHICS_CONTROL_BUTTON,
        .data.button = {action},
    };
    (*count) += 1;
}

static void
layoutControlRow(Graphics_Control *controls, int count, Graphics_Box box) {
    const int totalWidth = count * CONTROL_WIDTH + (count - 1) * CONTROL_GAP;
    const Graphics_Box row = Graphics_positionWithin(
        box,
        (Graphics_Size){totalWidth, 1},
        GRAPHICS_ALIGN_START,
        GRAPHICS_ALIGN_CENTER
    );

    Graphics_arrangeControlsWithin(
        row,
        GRAPHICS_DIRECTION_ROW,
        CONTROL_GAP,
        controls,
        count
    );
}

static void
layoutOptionsRow(Graphics_Control *controls, int count, Graphics_Box box) {
    if(count <= 0) {
        return;
    }

    if(count > 1) {
        layoutControlRow(controls, count - 1, box);
    }

    Graphics_positionControlWithin(
        &controls[count - 1],
        box,
        GRAPHICS_ALIGN_END,
        GRAPHICS_ALIGN_STRETCH
    );
}

Graphics_ControlGroup playControls(Runtime_Game *game, Graphics_Box box) {
    static Graphics_Control controls[8];
    Graphics_Box sections[4] = {
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 1},
    };
    int count = 0;
    int actionCount;

    (void)game;

    addAction(controls, &count, "HIT", 'h', noop);
    addAction(controls, &count, "STAND", 's', noop);
    addAction(controls, &count, "SPLIT", 'p', noop);
    addAction(controls, &count, "DOUBLE", 'd', noop);
    addAction(controls, &count, "SURRENDER", 'r', noop);
    actionCount = count;
    addAction(controls, &count, "SETTINGS", 'o', openSettings);
    addAction(controls, &count, "GUIDE", 'g', openGuide);
    addAction(controls, &count, "BACK", 'b', quitPlay);

    Graphics_arrangeWithin(box, GRAPHICS_DIRECTION_COLUMN, 0, sections, 4);
    layoutControlRow(controls, actionCount, sections[1]);
    layoutOptionsRow(&controls[actionCount], count - actionCount, sections[3]);

    for(int index = 0; index < count; index += 1) {
        controls[index].tabIndex = index;
    }

    const Graphics_ControlGroup group = {
        controls,
        count,
        &game->state.modeFocus[MODE_PLAY],
    };

    return group;
}
