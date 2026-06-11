#include "controls.h"

#include "modes/play/game.h"

enum { CONTROL_GAP = 2, CONTROL_WIDTH = 14 };

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
        .labelAlignment = GRAPHICS_ALIGN_CENTER,
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
        GRAPHICS_ALIGN_CENTER,
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

Graphics_ControlGroup playControls(Runtime_Game *game, Graphics_Box box) {
    static Graphics_Control controls[8];
    Graphics_Box rows[2] = {
        {0, 0, 0, 1},
        {0, 0, 0, 1},
    };
    int count = 0;
    int actionCount;
    PlayRound *round = &game->playRound;

    if(round->phase == ROUND_COMPLETE) {
        addAction(controls, &count, "NEXT", 'n', startNextPlayRound);
    } else if(round->phase == ROUND_PLAYER_TURN) {
        if(canHit(game)) {
            addAction(controls, &count, "HIT", 'h', playHit);
        }

        addAction(controls, &count, "STAND", 's', playStand);

        if(canDouble(game)) {
            addAction(controls, &count, "DOUBLE", 'd', playDouble);
        }

        if(canSplit(game)) {
            addAction(controls, &count, "SPLIT", 'p', playSplit);
        }

        if(canSurrender(game)) {
            addAction(controls, &count, "SURRENDER", 'r', playSurrender);
        }
    }

    actionCount = count;
    addAction(controls, &count, "SETTINGS", 'o', openSettings);
    addAction(controls, &count, "GUIDE", 'g', openGuide);
    addAction(controls, &count, "QUIT", 'q', quitPlay);

    Graphics_arrangeWithin(box, GRAPHICS_DIRECTION_COLUMN, 1, rows, 2);
    layoutControlRow(controls, actionCount, rows[0]);
    layoutControlRow(&controls[actionCount], count - actionCount, rows[1]);

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
