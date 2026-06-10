#include "runtime/runtime.h"

#include "modes/play/game.h"
#include "table/table.h"

Runtime_Game Runtime_initGame(void) {
    PlaySettings playSettings = Runtime_defaultPlaySettings();
    (void)Runtime_loadSettings(&playSettings);
    (void)Runtime_saveSettings(&playSettings);

    Runtime_Game game = {
        .state = Runtime_initState(),
        .playSettings = playSettings,
        .shoe = {0},
        .playRound = {0},
    };

    Table_initShoe(
        &game.shoe,
        game.playSettings.deckCount,
        game.playSettings.cutPercent
    );
    initPlayRound(&game);

    return game;
}
