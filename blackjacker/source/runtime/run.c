#include "runtime/runtime.h"

#include <curses.h>

#include "graphics/graphics.h"
#include "modes/guide/mode.h"
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
    Runtime_registerState(guideCallback, MODE_GUIDE);
}

static Runtime_Mode activeMode(Runtime_Game *game) {
    const Graphics_Box screen = Graphics_screenBox();

    if(screen.width < MIN_TERMINAL_WIDTH
        || screen.height < MIN_TERMINAL_HEIGHT) {
        return MODE_NOTICE;
    }

    return game->state.mode;
}

static const char *modeName(Runtime_Mode mode) {
    switch(mode) {
    case MODE_NOTICE:
        return "NOTICE";
    case MODE_MENU:
        return "MENU";
    case MODE_SETTINGS:
        return "SETTINGS";
    case MODE_PLAY:
        return "TRAIN";
    case MODE_GUIDE:
        return "GUIDE";
    case MODE_COUNT:
    default:
        return "";
    }
}

void Runtime_runGame(Runtime_Game *game) {
    registerStates();

    while(game->state.running) {
        int input;

        Runtime_applyQueuedState(&game->state);

        Runtime_Mode mode = activeMode(game);
        Runtime_ModeCallback callback = Runtime_stateCallback(mode);

        clear();

        if(callback) {
            callback(
                game,
                (Runtime_ModeEvent){
                    .type = MODE_EVENT_RENDER,
                }
            );
        }

        Graphics_drawMinimumFrame();
        Graphics_drawFrameChrome(modeName(mode));
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
