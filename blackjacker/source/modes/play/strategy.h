#ifndef BLACKJACKER_MODES_PLAY_STRATEGY_H
#define BLACKJACKER_MODES_PLAY_STRATEGY_H

#include "runtime/runtime.h"

/** Player actions recommended by the play strategy engine. */
typedef enum {
    STRATEGY_HIT,
    STRATEGY_STAND,
    STRATEGY_DOUBLE,
    STRATEGY_SPLIT,
    STRATEGY_SURRENDER,
} StrategyAction;

/** Returns a display label for a strategy action. */
const char *strategyActionLabel(StrategyAction action);

/**
 * Returns the rule-aware basic-strategy action for a player hand.
 *
 * The result is a legal action under the current game settings. When the
 * preferred basic-strategy action is unavailable, such as doubling after a
 * split when the table forbids it, the engine returns the appropriate hit or
 * stand fallback.
 */
StrategyAction perfectStrategyAction(Runtime_Game *game, int handIndex);

/**
 * Returns a rule-aware basic-strategy action for an arbitrary visible hand.
 *
 * This is useful for simulated table seats that are not stored in the primary
 * player hand array. The boolean flags describe which optional actions are
 * legal for that hand.
 */
StrategyAction
perfectStrategyHandAction(Runtime_Game *game, PlayHand hand, int handCount);

/**
 * Returns a rule-aware basic-strategy action for raw hand cards.
 *
 * This convenience wrapper is useful for callers that do not own a full
 * PlayHand instance.
 */
StrategyAction perfectStrategyCardsAction(
    Runtime_Game *game,
    Table_Hand cards,
    bool fromSplit,
    bool splitAces,
    bool allowSurrender,
    bool allowSplit
);

#endif
