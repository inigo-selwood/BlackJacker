#include "runtime/runtime.h"

#include <curses.h>

#include "graphics/graphics.h"
#include "modes/menu/mode.h"
#include "modes/notice/mode.h"
#include "modes/play/mode.h"
#include "modes/settings/mode.h"

static void registerStates(void) {
    Runtime_clearStateRegistry();
    Runtime_registerState(noticeCallback, MODE_NOTICE);
    Runtime_registerState(mainMenuCallback, MODE_MENU);
    Runtime_registerState(settingsCallback, MODE_SETTINGS);
    Runtime_registerState(playCallback, MODE_PLAY);
}

static Runtime_Mode activeMode(Runtime_Game *game) {
    const Graphics_Box screen = Graphics_screenBox();

    if(screen.width < MIN_TERMINAL_WIDTH
        || screen.height < MIN_TERMINAL_HEIGHT) {
        return MODE_NOTICE;
    }

    return game->state.mode;
}

void Runtime_runGame(Runtime_Game *game) {
    registerStates();

    while(game->state.running) {
        int input;

        Runtime_applyQueuedState(&game->state);

        Runtime_ModeCallback callback =
            Runtime_stateCallback(activeMode(game));

        clear();

        if(callback) {
            callback(
                game,
                (Runtime_ModeEvent){
                    .type = MODE_EVENT_RENDER,
                }
            );
        }

        refresh();

        input = getch();

        if(callback) {
            callback(
                game,
                (Runtime_ModeEvent){
                    .type = MODE_EVENT_INPUT,
                    .input = input,
                }
            );
        }
    }
}
