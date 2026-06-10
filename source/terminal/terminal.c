#include "terminal/terminal.h"

#include <locale.h>

enum { CARD_BACK_COLOR = 1 };

static void initTerminalColors(void) {
    if(!has_colors()) {
        return;
    }

    start_color();
    use_default_colors();
    init_pair(CARD_BACK_COLOR, -1, COLOR_RED);
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
