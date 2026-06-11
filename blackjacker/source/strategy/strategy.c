#include "strategy.h"

#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

#include "table/table.h"

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

typedef struct {
    Table_Hand hand;
    bool fromSplit;
    bool splitAces;
    bool allowSurrender;
    bool allowSplit;
    int handCount;
} StrategyHandContext;

static int rankStrategyValue(Table_Rank rank) {
    if(rank == TABLE_RANK_ACE) {
        return 11;
    }

    if(rank >= TABLE_RANK_TEN) {
        return 10;
    }

    return (int)rank + 1;
}

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

static const char *actionCode(const char *action) {
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
                actionCode(actionText)
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

static int dealerUpcardValue(Runtime_Game *game) {
    Table_Hand dealerHand = game->playRound.dealerHand;

    if(dealerHand.count <= 0) {
        return 0;
    }

    return rankStrategyValue(dealerHand.cards[0].rank);
}

static bool handIsPair(Table_Hand hand) {
    return hand.count == 2 && hand.cards[0].rank == hand.cards[1].rank;
}

static bool handIsAcePair(Table_Hand hand) {
    return handIsPair(hand) && hand.cards[0].rank == TABLE_RANK_ACE;
}

static bool handCanDouble(Runtime_Game *game, StrategyHandContext context) {
    const int value = Table_handValue(context.hand);

    if(game->playRound.phase != ROUND_PLAYER_TURN || context.hand.count != 2
        || !game->playSettings.allowDoubleDown || context.splitAces) {
        return false;
    }

    if(context.fromSplit && !game->playSettings.allowDoubleAfterSplit) {
        return false;
    }

    if(game->playSettings.doubleRule == DOUBLE_9_10_11) {
        return value >= 9 && value <= 11;
    }

    if(game->playSettings.doubleRule == DOUBLE_10_11) {
        return value == 10 || value == 11;
    }

    return true;
}

static bool handCanSplit(Runtime_Game *game, StrategyHandContext context) {
    Table_Hand hand = context.hand;

    if(game->playRound.phase != ROUND_PLAYER_TURN || !context.allowSplit
        || !game->playSettings.allowSplit
        || context.handCount >= MAX_PLAYER_HANDS || !handIsPair(hand)) {
        return false;
    }

    if(context.fromSplit && !game->playSettings.allowResplit) {
        return false;
    }

    return !handIsAcePair(hand) || !context.splitAces
        || game->playSettings.allowResplitAces;
}

static bool handCanSurrender(Runtime_Game *game, StrategyHandContext context) {
    return game->playRound.phase == ROUND_PLAYER_TURN
        && game->playSettings.allowSurrender && context.allowSurrender
        && context.hand.count == 2 && context.handCount == 1;
}

static Strategy_Action doubleOrFallback(
    Runtime_Game *game,
    StrategyHandContext context,
    Strategy_Action fallback
) {
    return handCanDouble(game, context) ? STRATEGY_ACTION_DOUBLE : fallback;
}

static Strategy_Action splitOrFallback(
    Runtime_Game *game,
    StrategyHandContext context,
    Strategy_Action fallback
) {
    return handCanSplit(game, context) ? STRATEGY_ACTION_SPLIT : fallback;
}

static Strategy_Action surrenderOrFallback(
    Runtime_Game *game,
    StrategyHandContext context,
    Strategy_Action fallback
) {
    return handCanSurrender(game, context) ? STRATEGY_ACTION_SURRENDER
                                           : fallback;
}

static Strategy_Action
hardStrategy(Runtime_Game *game, StrategyHandContext context) {
    Table_Hand hand = context.hand;
    const int value = Table_handValue(hand);
    const int dealer = dealerUpcardValue(game);
    const bool dealerHitsSoft17 =
        game->playSettings.dealerSoft17Rule == DEALER_HITS_SOFT_17;

    if(value >= 17) {
        return STRATEGY_ACTION_STAND;
    }

    if(value == 16) {
        if(dealer >= 9) {
            return surrenderOrFallback(game, context, STRATEGY_ACTION_HIT);
        }

        return dealer >= 2 && dealer <= 6 ? STRATEGY_ACTION_STAND
                                          : STRATEGY_ACTION_HIT;
    }

    if(value == 15) {
        if(dealer == 10 || (dealer == 11 && dealerHitsSoft17)) {
            return surrenderOrFallback(game, context, STRATEGY_ACTION_HIT);
        }

        return dealer >= 2 && dealer <= 6 ? STRATEGY_ACTION_STAND
                                          : STRATEGY_ACTION_HIT;
    }

    if(value >= 13) {
        return dealer >= 2 && dealer <= 6 ? STRATEGY_ACTION_STAND
                                          : STRATEGY_ACTION_HIT;
    }

    if(value == 12) {
        return dealer >= 4 && dealer <= 6 ? STRATEGY_ACTION_STAND
                                          : STRATEGY_ACTION_HIT;
    }

    if(value == 11) {
        if(dealer == 11 && !dealerHitsSoft17) {
            return STRATEGY_ACTION_HIT;
        }

        return doubleOrFallback(game, context, STRATEGY_ACTION_HIT);
    }

    if(value == 10) {
        return dealer >= 2 && dealer <= 9
            ? doubleOrFallback(game, context, STRATEGY_ACTION_HIT)
            : STRATEGY_ACTION_HIT;
    }

    if(value == 9) {
        return dealer >= 3 && dealer <= 6
            ? doubleOrFallback(game, context, STRATEGY_ACTION_HIT)
            : STRATEGY_ACTION_HIT;
    }

    return STRATEGY_ACTION_HIT;
}

static Strategy_Action
softStrategy(Runtime_Game *game, StrategyHandContext context) {
    Table_Hand hand = context.hand;
    const int value = Table_handValue(hand);
    const int dealer = dealerUpcardValue(game);
    const bool dealerHitsSoft17 =
        game->playSettings.dealerSoft17Rule == DEALER_HITS_SOFT_17;

    if(value >= 20) {
        return STRATEGY_ACTION_STAND;
    }

    if(value == 19) {
        if(dealer == 6 && dealerHitsSoft17) {
            return doubleOrFallback(game, context, STRATEGY_ACTION_STAND);
        }

        return STRATEGY_ACTION_STAND;
    }

    if(value == 18) {
        if(dealer >= 3 && dealer <= 6) {
            return doubleOrFallback(game, context, STRATEGY_ACTION_STAND);
        }

        if(dealer == 2 && dealerHitsSoft17) {
            return doubleOrFallback(game, context, STRATEGY_ACTION_STAND);
        }

        return dealer == 7 || dealer == 8 ? STRATEGY_ACTION_STAND
                                          : STRATEGY_ACTION_HIT;
    }

    if(value == 17) {
        if(dealer >= 3 && dealer <= 6) {
            return doubleOrFallback(game, context, STRATEGY_ACTION_HIT);
        }

        if(dealer == 2 && dealerHitsSoft17) {
            return doubleOrFallback(game, context, STRATEGY_ACTION_HIT);
        }

        return STRATEGY_ACTION_HIT;
    }

    if(value == 16 || value == 15) {
        if(dealer >= 4 && dealer <= 6) {
            return doubleOrFallback(game, context, STRATEGY_ACTION_HIT);
        }

        return STRATEGY_ACTION_HIT;
    }

    if(value == 14 || value == 13) {
        if(dealer >= 5 && dealer <= 6) {
            return doubleOrFallback(game, context, STRATEGY_ACTION_HIT);
        }

        if(dealer == 4 && dealerHitsSoft17) {
            return doubleOrFallback(game, context, STRATEGY_ACTION_HIT);
        }
    }

    return STRATEGY_ACTION_HIT;
}

static Strategy_Action
pairStrategy(Runtime_Game *game, StrategyHandContext context) {
    Table_Hand hand = context.hand;
    const Table_Rank rank = hand.cards[0].rank;
    const int dealer = dealerUpcardValue(game);
    const bool doubleAfterSplit = game->playSettings.allowDoubleAfterSplit;

    switch(rank) {
    case TABLE_RANK_ACE:
    case TABLE_RANK_EIGHT:
        return splitOrFallback(game, context, hardStrategy(game, context));
    case TABLE_RANK_TEN:
    case TABLE_RANK_JACK:
    case TABLE_RANK_QUEEN:
    case TABLE_RANK_KING:
        return STRATEGY_ACTION_STAND;
    case TABLE_RANK_NINE:
        if((dealer >= 2 && dealer <= 6) || dealer == 8 || dealer == 9) {
            return splitOrFallback(game, context, STRATEGY_ACTION_STAND);
        }
        return STRATEGY_ACTION_STAND;
    case TABLE_RANK_SEVEN:
        if(dealer >= 2 && dealer <= 7) {
            return splitOrFallback(game, context, STRATEGY_ACTION_HIT);
        }
        return STRATEGY_ACTION_HIT;
    case TABLE_RANK_SIX:
        if((dealer >= 3 && dealer <= 6) || (dealer == 2 && doubleAfterSplit)) {
            return splitOrFallback(game, context, STRATEGY_ACTION_HIT);
        }
        return STRATEGY_ACTION_HIT;
    case TABLE_RANK_FIVE:
        return hardStrategy(game, context);
    case TABLE_RANK_FOUR:
        if(doubleAfterSplit && dealer >= 5 && dealer <= 6) {
            return splitOrFallback(game, context, STRATEGY_ACTION_HIT);
        }
        return STRATEGY_ACTION_HIT;
    case TABLE_RANK_THREE:
    case TABLE_RANK_TWO:
        if((dealer >= 4 && dealer <= 7)
            || (doubleAfterSplit && dealer >= 2 && dealer <= 3)) {
            return splitOrFallback(game, context, STRATEGY_ACTION_HIT);
        }
        return STRATEGY_ACTION_HIT;
    default:
        return hardStrategy(game, context);
    }
}

const char *strategyActionLabel(Strategy_Action action) {
    switch(action) {
    case STRATEGY_ACTION_STAND:
        return "Stand";
    case STRATEGY_ACTION_DOUBLE:
        return "Double";
    case STRATEGY_ACTION_SPLIT:
        return "Split";
    case STRATEGY_ACTION_SURRENDER:
        return "Surrender";
    case STRATEGY_ACTION_HIT:
    default:
        return "Hit";
    }
}

const char *Strategy_actionCode(Strategy_Action action) {
    switch(action) {
    case STRATEGY_ACTION_STAND:
        return "S";
    case STRATEGY_ACTION_DOUBLE:
        return "D";
    case STRATEGY_ACTION_SPLIT:
        return "P";
    case STRATEGY_ACTION_SURRENDER:
        return "R";
    case STRATEGY_ACTION_HIT:
    default:
        return "H";
    }
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

static Strategy_Action
strategyForContext(Runtime_Game *game, StrategyHandContext context) {
    if(!game || context.hand.count <= 0
        || game->playRound.phase != ROUND_PLAYER_TURN) {
        return STRATEGY_ACTION_STAND;
    }

    if(handIsPair(context.hand)) {
        return pairStrategy(game, context);
    }

    if(Table_handIsSoft(context.hand)) {
        return softStrategy(game, context);
    }

    return hardStrategy(game, context);
}

Strategy_Action perfectStrategyAction(Runtime_Game *game, int handIndex) {
    StrategyHandContext context;

    if(!game || handIndex < 0
        || handIndex >= game->playRound.playerHandCount) {
        return STRATEGY_ACTION_STAND;
    }

    context = (StrategyHandContext){
        .hand = game->playRound.playerHands[handIndex].cards,
        .fromSplit = game->playRound.playerHandCount > 1,
        .splitAces = game->playRound.playerHands[handIndex].splitAces,
        .allowSurrender =
            game->playRound.playerHands[handIndex].decision.allowSurrender,
        .allowSplit =
            game->playRound.playerHands[handIndex].decision.allowSplit,
        .handCount = game->playRound.playerHandCount,
    };

    return strategyForContext(game, context);
}

Strategy_Action
perfectStrategyHandAction(Runtime_Game *game, PlayHand hand, int handCount) {
    const StrategyHandContext context = {
        .hand = hand.cards,
        .fromSplit = handCount > 1,
        .splitAces = hand.splitAces,
        .allowSurrender = hand.decision.allowSurrender,
        .allowSplit = hand.decision.allowSplit,
        .handCount = handCount,
    };

    return strategyForContext(game, context);
}

Strategy_Action perfectStrategyCardsAction(
    Runtime_Game *game,
    Table_Hand cards,
    bool fromSplit,
    bool splitAces,
    bool allowSurrender,
    bool allowSplit
) {
    const StrategyHandContext context = {
        .hand = cards,
        .fromSplit = fromSplit,
        .splitAces = splitAces,
        .allowSurrender = allowSurrender,
        .allowSplit = allowSplit,
        .handCount = fromSplit ? 2 : 1,
    };

    return strategyForContext(game, context);
}
