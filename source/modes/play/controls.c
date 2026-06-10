#include "controls.h"

#include "modes/play/game.h"

enum { CARD_GAP = 2, CONTROL_WIDTH = 11 };

static void quitPlay(Runtime_Game *game) {
    Runtime_queueState(&game->state, MODE_MENU);
}

static void addAction(
    Graphics_Control *controls,
    int *count,
    const char *label,
    Graphics_ButtonAction action
) {
    controls[*count] = (Graphics_Control){
        .label = label,
        .labelWidth = 0,
        .labelAlignment = GRAPHICS_ALIGN_CENTER,
        .width = CONTROL_WIDTH,
        .type = GRAPHICS_CONTROL_BUTTON,
        .data.button = {action},
    };
    (*count) += 1;
}

static void
layoutControlRow(Graphics_Control *controls, int count, Graphics_Box box) {
    const int totalWidth = count * CONTROL_WIDTH + (count - 1) * CARD_GAP;
    const Graphics_Box row = Graphics_positionWithin(
        box,
        (Graphics_Size){totalWidth, 1},
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_CENTER
    );

    Graphics_arrangeControlsWithin(
        row,
        GRAPHICS_DIRECTION_ROW,
        CARD_GAP,
        controls,
        count
    );
}

Graphics_ControlGroup playControls(Runtime_Game *game, Graphics_Box box) {
    static Graphics_Control controls[6];
    int count = 0;
    PlayRound *round = &game->playRound;

    if(round->phase == ROUND_COMPLETE) {
        addAction(controls, &count, "NEXT ROUND", startNextPlayRound);
    } else if(round->phase == ROUND_PLAYER_TURN) {
        if(canHit(game)) {
            addAction(controls, &count, "HIT", playHit);
        }

        addAction(controls, &count, "STAND", playStand);

        if(canDouble(game)) {
            addAction(controls, &count, "DOUBLE", playDouble);
        }

        if(canSplit(game)) {
            addAction(controls, &count, "SPLIT", playSplit);
        }

        if(canSurrender(game)) {
            addAction(controls, &count, "SURRENDER", playSurrender);
        }
    }

    addAction(controls, &count, "QUIT", quitPlay);
    layoutControlRow(controls, count, box);

    const Graphics_ControlGroup group = {
        controls,
        count,
        &game->state.modeFocus[MODE_PLAY],
    };

    return group;
}
