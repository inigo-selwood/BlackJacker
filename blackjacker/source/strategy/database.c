#include "strategy.h"

#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

enum {
    STRATEGY_HAND_KIND_COUNT = 3,
    STRATEGY_TOTAL_COUNT = 23,
    STRATEGY_DEALER_UPCARD_COUNT = 10,
};

typedef struct {
    bool available;
    double ev;
    Strategy_Action action;
    char actionCode[5];
} StrategyBestAction;

static StrategyBestAction bestActions[STRATEGY_HAND_KIND_COUNT]
                                     [STRATEGY_TOTAL_COUNT]
                                     [STRATEGY_DEALER_UPCARD_COUNT];
static bool databaseLoaded = false;

static int dealerUpcardIndex(Table_Rank rank) {
    if(rank == TABLE_RANK_ACE) {
        return 9;
    }

    if(rank >= TABLE_RANK_TEN) {
        return 8;
    }

    return (int)rank - 1;
}

static bool parseHandKind(const char *text, Strategy_HandKind *handKind) {
    if(strcmp(text, "hard") == 0) {
        *handKind = STRATEGY_HAND_KIND_HARD;
        return true;
    }

    if(strcmp(text, "soft") == 0) {
        *handKind = STRATEGY_HAND_KIND_SOFT;
        return true;
    }

    if(strcmp(text, "pair") == 0) {
        *handKind = STRATEGY_HAND_KIND_PAIR;
        return true;
    }

    return false;
}

static bool parseDealerUpcard(const char *text, Table_Rank *rank) {
    if(strcmp(text, "A") == 0 || strcmp(text, "ace") == 0) {
        *rank = TABLE_RANK_ACE;
        return true;
    }

    if(strcmp(text, "10") == 0) {
        *rank = TABLE_RANK_TEN;
        return true;
    }

    if(strlen(text) == 1 && text[0] >= '2' && text[0] <= '9') {
        *rank = (Table_Rank)(text[0] - '1');
        return true;
    }

    return false;
}

static bool parseAction(const char *text, Strategy_Action *action) {
    if(strcmp(text, "hit") == 0) {
        *action = STRATEGY_ACTION_HIT;
        return true;
    }

    if(strcmp(text, "stand") == 0) {
        *action = STRATEGY_ACTION_STAND;
        return true;
    }

    if(strncmp(text, "double", 6) == 0) {
        *action = STRATEGY_ACTION_DOUBLE;
        return true;
    }

    if(strncmp(text, "split", 5) == 0) {
        *action = STRATEGY_ACTION_SPLIT;
        return true;
    }

    if(strncmp(text, "surrender", 9) == 0) {
        *action = STRATEGY_ACTION_SURRENDER;
        return true;
    }

    return false;
}

static const char *generatedActionCode(const char *action) {
    if(strcmp(action, "stand") == 0) {
        return "S";
    }

    if(strcmp(action, "double-hit") == 0) {
        return "D/H";
    }

    if(strcmp(action, "double-stand") == 0) {
        return "D/S";
    }

    if(strcmp(action, "double") == 0) {
        return "D";
    }

    if(strcmp(action, "split-hit") == 0) {
        return "P/H";
    }

    if(strcmp(action, "split-stand") == 0) {
        return "P/S";
    }

    if(strcmp(action, "split") == 0) {
        return "P";
    }

    if(strcmp(action, "surrender-hit") == 0) {
        return "R/H";
    }

    if(strcmp(action, "surrender-stand") == 0) {
        return "R/S";
    }

    if(strcmp(action, "surrender") == 0) {
        return "R";
    }

    return "H";
}

static bool shouldReplaceBestAction(
    StrategyBestAction current,
    double ev,
    Strategy_Action action
) {
    if(!current.available) {
        return true;
    }

    if(ev > current.ev) {
        return true;
    }

    if(ev < current.ev) {
        return false;
    }

    return action < current.action;
}

static void clearBestActions(void) {
    for(int handKind = 0; handKind < STRATEGY_HAND_KIND_COUNT; handKind += 1) {
        for(int total = 0; total < STRATEGY_TOTAL_COUNT; total += 1) {
            for(int upcard = 0; upcard < STRATEGY_DEALER_UPCARD_COUNT;
                upcard += 1) {
                bestActions[handKind][total][upcard] = (StrategyBestAction){
                    .available = false,
                };
            }
        }
    }
}

static bool loadDatabasePath(const char *path) {
    sqlite3 *database = 0;
    sqlite3_stmt *statement = 0;
    const char *sql =
        "select hand_kind, player_total, dealer_upcard, action, ev "
        "from expected_values where available = 1";
    int result;
    bool ok = true;

    if(sqlite3_open_v2(path, &database, SQLITE_OPEN_READONLY, 0)
        != SQLITE_OK) {
        sqlite3_close(database);
        return false;
    }

    if(sqlite3_prepare_v2(database, sql, -1, &statement, 0) != SQLITE_OK) {
        sqlite3_close(database);
        return false;
    }

    while((result = sqlite3_step(statement)) == SQLITE_ROW) {
        Strategy_HandKind handKind;
        Strategy_Action action;
        Table_Rank dealerUpcard;
        const char *handKindText =
            (const char *)sqlite3_column_text(statement, 0);
        const int playerTotal = sqlite3_column_int(statement, 1);
        const char *dealerUpcardText =
            (const char *)sqlite3_column_text(statement, 2);
        const char *actionText =
            (const char *)sqlite3_column_text(statement, 3);
        const double ev = sqlite3_column_double(statement, 4);
        int upcardIndex;
        StrategyBestAction *current;

        if(!handKindText || !dealerUpcardText || !actionText || playerTotal < 0
            || playerTotal >= STRATEGY_TOTAL_COUNT
            || !parseHandKind(handKindText, &handKind)
            || !parseDealerUpcard(dealerUpcardText, &dealerUpcard)
            || !parseAction(actionText, &action)) {
            continue;
        }

        upcardIndex = dealerUpcardIndex(dealerUpcard);
        current = &bestActions[handKind][playerTotal][upcardIndex];

        if(shouldReplaceBestAction(*current, ev, action)) {
            *current = (StrategyBestAction){
                .available = true,
                .ev = ev,
                .action = action,
            };
            snprintf(
                current->actionCode,
                sizeof(current->actionCode),
                "%s",
                generatedActionCode(actionText)
            );
        }
    }

    if(result != SQLITE_DONE) {
        ok = false;
    }

    sqlite3_finalize(statement);
    sqlite3_close(database);
    return ok;
}

bool Strategy_loadDatabase(void) {
    if(databaseLoaded) {
        return true;
    }

    clearBestActions();

    databaseLoaded = loadDatabasePath("../resources/strategy.sqlite3")
        || loadDatabasePath("resources/strategy.sqlite3");

    return databaseLoaded;
}

bool Strategy_databaseLoaded(void) {
    return databaseLoaded;
}

bool Strategy_bestAction(
    Strategy_HandKind handKind,
    int playerTotal,
    Table_Rank dealerUpcard,
    Strategy_Action *action
) {
    const int upcardIndex = dealerUpcardIndex(dealerUpcard);
    StrategyBestAction bestAction;

    if(!action || !Strategy_loadDatabase() || handKind < 0
        || handKind >= STRATEGY_HAND_KIND_COUNT || playerTotal < 0
        || playerTotal >= STRATEGY_TOTAL_COUNT || upcardIndex < 0
        || upcardIndex >= STRATEGY_DEALER_UPCARD_COUNT) {
        return false;
    }

    bestAction = bestActions[handKind][playerTotal][upcardIndex];

    if(!bestAction.available) {
        return false;
    }

    *action = bestAction.action;
    return true;
}

bool Strategy_bestActionCode(
    Strategy_HandKind handKind,
    int playerTotal,
    Table_Rank dealerUpcard,
    const char **code
) {
    const int upcardIndex = dealerUpcardIndex(dealerUpcard);
    StrategyBestAction bestAction;

    if(!code || !Strategy_loadDatabase() || handKind < 0
        || handKind >= STRATEGY_HAND_KIND_COUNT || playerTotal < 0
        || playerTotal >= STRATEGY_TOTAL_COUNT || upcardIndex < 0
        || upcardIndex >= STRATEGY_DEALER_UPCARD_COUNT) {
        return false;
    }

    bestAction = bestActions[handKind][playerTotal][upcardIndex];

    if(!bestAction.available || bestAction.actionCode[0] == '\0') {
        return false;
    }

    *code = bestActions[handKind][playerTotal][upcardIndex].actionCode;
    return true;
}
