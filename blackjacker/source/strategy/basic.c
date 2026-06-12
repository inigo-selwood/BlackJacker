#include "strategy.h"

#include "table/table.h"

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
