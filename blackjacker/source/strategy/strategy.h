#ifndef BLACKJACKER_STRATEGY_H
#define BLACKJACKER_STRATEGY_H

#include "runtime/runtime.h"

/** Player actions recommended by the play strategy engine. */
typedef enum {
    STRATEGY_ACTION_HIT,
    STRATEGY_ACTION_STAND,
    STRATEGY_ACTION_DOUBLE,
    STRATEGY_ACTION_SPLIT,
    STRATEGY_ACTION_SURRENDER,
} Strategy_Action;

/** Strategy-table hand categories. */
typedef enum {
    STRATEGY_HAND_KIND_HARD,
    STRATEGY_HAND_KIND_SOFT,
    STRATEGY_HAND_KIND_PAIR,
} Strategy_HandKind;

/** Returns a display label for a strategy action. */
const char *strategyActionLabel(Strategy_Action action);

/** Returns a compact display code for a strategy action. */
const char *Strategy_actionCode(Strategy_Action action);

/** Loads and caches best-action rows from the generated SQLite database. */
bool Strategy_loadDatabase(void);

/** Returns true when the generated strategy database is loaded. */
bool Strategy_databaseLoaded(void);

/** Returns the best available action for a guide context, or false. */
bool Strategy_bestAction(
    Strategy_HandKind handKind,
    int playerTotal,
    Table_Rank dealerUpcard,
    Strategy_Action *action
);

/** Returns the generated guide-table code for a context, or false. */
bool Strategy_bestActionCode(
    Strategy_HandKind handKind,
    int playerTotal,
    Table_Rank dealerUpcard,
    const char **code
);

/**
 * Returns the rule-aware basic-strategy action for a player hand.
 *
 * The result is a legal action under the current game settings. When the
 * preferred basic-strategy action is unavailable, such as doubling after a
 * split when the table forbids it, the engine returns the appropriate hit or
 * stand fallback.
 */
Strategy_Action perfectStrategyAction(Runtime_Game *game, int handIndex);

/**
 * Returns a rule-aware basic-strategy action for an arbitrary visible hand.
 *
 * This is useful for simulated table seats that are not stored in the primary
 * player hand array. The boolean flags describe which optional actions are
 * legal for that hand.
 */
Strategy_Action
perfectStrategyHandAction(Runtime_Game *game, PlayHand hand, int handCount);

/**
 * Returns a rule-aware basic-strategy action for raw hand cards.
 *
 * This convenience wrapper is useful for callers that do not own a full
 * PlayHand instance.
 */
Strategy_Action perfectStrategyCardsAction(
    Runtime_Game *game,
    Table_Hand cards,
    bool fromSplit,
    bool splitAces,
    bool allowSurrender,
    bool allowSplit
);

#endif
