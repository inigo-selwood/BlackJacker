#include "runtime/runtime.h"
#include "terminal/terminal.h"

int main(void) {
    Runtime_Game game = Runtime_initGame();

    Terminal_init();
    Runtime_runGame(&game);
    endwin();

    return 0;
}
