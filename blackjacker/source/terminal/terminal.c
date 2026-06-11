#include "terminal/terminal.h"

#include <locale.h>

enum {
    CARD_BACK_COLOR = 1,
    ACTION_HIT_COLOR,
    ACTION_STAND_COLOR,
    ACTION_DOUBLE_COLOR,
    ACTION_SPLIT_COLOR,
    ACTION_SURRENDER_COLOR,
};

static void initTerminalColors(void) {
    if(!has_colors()) {
        return;
    }

    start_color();
    use_default_colors();
    init_pair(CARD_BACK_COLOR, -1, COLOR_RED);
    init_pair(ACTION_HIT_COLOR, COLOR_BLACK, COLOR_BLUE);
    init_pair(ACTION_STAND_COLOR, COLOR_BLACK, COLOR_GREEN);
    init_pair(ACTION_DOUBLE_COLOR, COLOR_BLACK, COLOR_YELLOW);
    init_pair(ACTION_SPLIT_COLOR, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(ACTION_SURRENDER_COLOR, COLOR_BLACK, COLOR_RED);
}

void Terminal_init(void) {
    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    initTerminalColors();
}

attr_t Terminal_cardBackAttrs(void) {
    return has_colors() ? COLOR_PAIR(CARD_BACK_COLOR) : A_NORMAL;
}

attr_t Terminal_actionAttrs(const char *actionCode) {
    if(!has_colors() || !actionCode) {
        return A_NORMAL;
    }

    switch(actionCode[0]) {
    case 'H':
        return COLOR_PAIR(ACTION_HIT_COLOR);
    case 'S':
        return COLOR_PAIR(ACTION_STAND_COLOR);
    case 'D':
        return COLOR_PAIR(ACTION_DOUBLE_COLOR);
    case 'P':
        return COLOR_PAIR(ACTION_SPLIT_COLOR);
    case 'R':
        return COLOR_PAIR(ACTION_SURRENDER_COLOR);
    default:
        return A_NORMAL;
    }
}
