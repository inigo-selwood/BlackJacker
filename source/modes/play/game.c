#include "game.h"

#include "modes/play/strategy.h"
#include "table/table.h"

static void clearHand(Table_Hand *hand) {
    hand->count = 0;
}

static void addCard(Runtime_Game *game, Table_Hand *hand) {
    if(hand->count >= TABLE_MAX_HAND_CARDS) {
        return;
    }

    hand->cards[hand->count] = Table_drawShoeCard(
        &game->shoe,
        game->playSettings.deckCount,
        game->playSettings.cutPercent
    );
    hand->count += 1;
}

static PlayDecision userDecision(void) {
    return (PlayDecision){
        .source = PLAY_DECISION_USER,
        .allowSurrender = true,
        .allowSplit = true,
    };
}

static PlayDecision strategyDecision(void) {
    return (PlayDecision){
        .source = PLAY_DECISION_STRATEGY,
        .allowSurrender = false,
        .allowSplit = false,
    };
}

static void resetPlayHand(PlayHand *hand, PlayDecision decision) {
    *hand = (PlayHand){
        .cards = {0},
        .result = ROUND_RESULT_NONE,
        .decision = decision,
        .doubledDown = false,
        .splitAces = false,
    };
}

static PlayAction strategyToPlayAction(StrategyAction action) {
    switch(action) {
    case STRATEGY_STAND:
        return PLAY_ACTION_STAND;
    case STRATEGY_DOUBLE:
        return PLAY_ACTION_DOUBLE;
    case STRATEGY_SPLIT:
        return PLAY_ACTION_SPLIT;
    case STRATEGY_SURRENDER:
        return PLAY_ACTION_SURRENDER;
    case STRATEGY_HIT:
    default:
        return PLAY_ACTION_HIT;
    }
}

static PlayAction decidePlayHandAction(
    Runtime_Game *game,
    PlayHand hand,
    int handCount,
    PlayAction userAction
) {
    if(hand.decision.source == PLAY_DECISION_USER) {
        return userAction;
    }

    if(hand.decision.source == PLAY_DECISION_STRATEGY) {
        return strategyToPlayAction(
            perfectStrategyHandAction(game, hand, handCount)
        );
    }

    return PLAY_ACTION_NONE;
}

static void playOtherPlayerHand(Runtime_Game *game, int handIndex) {
    PlayRound *round = &game->playRound;
    PlayHand *playHand = &round->otherPlayerHands[handIndex];
    Table_Hand *hand = &playHand->cards;

    while(Table_handValue(*hand) <= 21) {
        const PlayAction action = decidePlayHandAction(
            game,
            *playHand,
            round->otherPlayerHandCount,
            PLAY_ACTION_NONE
        );

        if(action != PLAY_ACTION_HIT && action != PLAY_ACTION_DOUBLE) {
            return;
        }

        addCard(game, hand);

        if(action == PLAY_ACTION_DOUBLE) {
            playHand->doubledDown = true;
            return;
        }
    }

    playHand->result = ROUND_RESULT_PLAYER_BUST;
}

static void playOtherPlayersBeforePlayer(Runtime_Game *game) {
    PlayRound *round = &game->playRound;

    for(int index = 0;
        index < round->otherPlayerHandCount && index < PLAYER_TABLE_SEAT_INDEX;
        index += 1) {
        playOtherPlayerHand(game, index);
    }
}

static void playOtherPlayersAfterPlayer(Runtime_Game *game) {
    PlayRound *round = &game->playRound;

    for(int index = PLAYER_TABLE_SEAT_INDEX;
        index < round->otherPlayerHandCount;
        index += 1) {
        playOtherPlayerHand(game, index);
    }
}

static bool dealerShouldHit(Runtime_Game *game) {
    const Table_Hand hand = game->playRound.dealerHand;
    const int value = Table_handValue(hand);

    if(value < 17) {
        return true;
    }

    return value == 17 && Table_handIsSoft(hand)
        && game->playSettings.dealerSoft17Rule == DEALER_HITS_SOFT_17;
}

static bool doubleRuleAllows(Runtime_Game *game, Table_Hand hand) {
    const int value = Table_handValue(hand);

    if(game->playSettings.doubleRule == DOUBLE_9_10_11) {
        return value >= 9 && value <= 11;
    }

    if(game->playSettings.doubleRule == DOUBLE_10_11) {
        return value == 10 || value == 11;
    }

    return true;
}

static void resolveHand(Runtime_Game *game, int handIndex) {
    PlayRound *round = &game->playRound;
    PlayHand *hand = &round->playerHands[handIndex];
    const int playerValue = Table_handValue(hand->cards);
    const int dealerValue = Table_handValue(round->dealerHand);

    if(hand->result == ROUND_RESULT_SURRENDER) {
        return;
    }

    if(playerValue > 21) {
        hand->result = ROUND_RESULT_PLAYER_BUST;
    } else if(dealerValue > 21) {
        hand->result = ROUND_RESULT_DEALER_BUST;
    } else if(playerValue > dealerValue) {
        hand->result = ROUND_RESULT_PLAYER_WIN;
    } else if(dealerValue > playerValue) {
        hand->result = ROUND_RESULT_DEALER_WIN;
    } else {
        hand->result = ROUND_RESULT_PUSH;
    }
}

static RoundResult resolveVisibleHand(Table_Hand hand, Table_Hand dealerHand) {
    const int playerValue = Table_handValue(hand);
    const int dealerValue = Table_handValue(dealerHand);

    if(playerValue > 21) {
        return ROUND_RESULT_PLAYER_BUST;
    }

    if(dealerValue > 21) {
        return ROUND_RESULT_DEALER_BUST;
    }

    if(playerValue > dealerValue) {
        return ROUND_RESULT_PLAYER_WIN;
    }

    if(dealerValue > playerValue) {
        return ROUND_RESULT_DEALER_WIN;
    }

    return ROUND_RESULT_PUSH;
}

static bool hasLiveDealerHand(Runtime_Game *game) {
    PlayRound *round = &game->playRound;

    for(int index = 0; index < round->playerHandCount; index += 1) {
        if(round->playerHands[index].result == ROUND_RESULT_NONE
            && Table_handValue(round->playerHands[index].cards) <= 21) {
            return true;
        }
    }

    for(int index = 0; index < round->otherPlayerHandCount; index += 1) {
        if(round->otherPlayerHands[index].result == ROUND_RESULT_NONE
            && Table_handValue(round->otherPlayerHands[index].cards) <= 21) {
            return true;
        }
    }

    return false;
}

static void completeRound(Runtime_Game *game) {
    PlayRound *round = &game->playRound;

    if(hasLiveDealerHand(game)) {
        if(game->playSettings.useNoHoleCardRule
            && round->dealerHand.count == 1) {
            addCard(game, &round->dealerHand);
        }

        while(dealerShouldHit(game)) {
            addCard(game, &round->dealerHand);
        }
    }

    for(int index = 0; index < round->playerHandCount; index += 1) {
        resolveHand(game, index);
    }

    for(int index = 0; index < round->otherPlayerHandCount; index += 1) {
        if(round->otherPlayerHands[index].result == ROUND_RESULT_NONE) {
            round->otherPlayerHands[index].result = resolveVisibleHand(
                round->otherPlayerHands[index].cards,
                round->dealerHand
            );
        }
    }

    round->phase = ROUND_COMPLETE;
}

static void finishActiveHand(Runtime_Game *game) {
    PlayRound *round = &game->playRound;

    if(round->activePlayerHand + 1 < round->playerHandCount) {
        round->activePlayerHand += 1;
        return;
    }

    playOtherPlayersAfterPlayer(game);
    completeRound(game);
}

static PlayAction
decideActivePlayerAction(Runtime_Game *game, PlayAction userAction) {
    PlayRound *round = &game->playRound;

    if(round->activePlayerHand < 0
        || round->activePlayerHand >= round->playerHandCount) {
        return PLAY_ACTION_NONE;
    }

    return decidePlayHandAction(
        game,
        round->playerHands[round->activePlayerHand],
        round->playerHandCount,
        userAction
    );
}

static bool activeHandHasPair(Runtime_Game *game) {
    Table_Hand *hand = activePlayerHand(game);

    return hand && hand->count == 2
        && hand->cards[0].rank == hand->cards[1].rank;
}

static bool activeHandIsSplitAces(Runtime_Game *game) {
    PlayRound *round = &game->playRound;

    return round->playerHands[round->activePlayerHand].splitAces;
}

static bool handIsAcePair(Table_Hand hand) {
    return hand.count == 2 && hand.cards[0].rank == TABLE_RANK_ACE
        && hand.cards[1].rank == TABLE_RANK_ACE;
}

static bool canActOnSplitAces(Runtime_Game *game) {
    return !activeHandIsSplitAces(game)
        || game->playSettings.allowHitSplitAces;
}

static bool splitLimitAllows(Runtime_Game *game) {
    PlayRound *round = &game->playRound;

    if(round->playerHandCount >= MAX_PLAYER_HANDS) {
        return false;
    }

    return round->playerHandCount == 1 || game->playSettings.allowResplit;
}

void initPlayRound(Runtime_Game *game) {
    game->playRound = (PlayRound){
        .dealerHand = {0},
        .otherPlayerHands = {{0}},
        .playerHands = {{0}},
        .roundNumber = 0,
        .otherPlayerHandCount = MAX_OTHER_PLAYER_HANDS,
        .playerHandCount = 1,
        .activePlayerHand = 0,
        .phase = ROUND_PLAYER_TURN,
    };
    startNextPlayRound(game);
}

void startNextPlayRound(Runtime_Game *game) {
    PlayRound *round = &game->playRound;

    clearHand(&round->dealerHand);

    for(int index = 0; index < MAX_OTHER_PLAYER_HANDS; index += 1) {
        resetPlayHand(&round->otherPlayerHands[index], strategyDecision());
    }

    for(int index = 0; index < MAX_PLAYER_HANDS; index += 1) {
        resetPlayHand(&round->playerHands[index], userDecision());
    }

    round->roundNumber += 1;
    round->otherPlayerHandCount = MAX_OTHER_PLAYER_HANDS;
    round->playerHandCount = 1;
    round->activePlayerHand = 0;
    round->phase = ROUND_PLAYER_TURN;

    for(int index = 0; index < round->otherPlayerHandCount; index += 1) {
        addCard(game, &round->otherPlayerHands[index].cards);
    }

    addCard(game, &round->playerHands[0].cards);
    addCard(game, &round->dealerHand);

    for(int index = 0; index < round->otherPlayerHandCount; index += 1) {
        addCard(game, &round->otherPlayerHands[index].cards);
    }

    addCard(game, &round->playerHands[0].cards);

    if(!game->playSettings.useNoHoleCardRule) {
        addCard(game, &round->dealerHand);
    }

    playOtherPlayersBeforePlayer(game);
}

void playHit(Runtime_Game *game) {
    Table_Hand *hand = activePlayerHand(game);

    if(decideActivePlayerAction(game, PLAY_ACTION_HIT) != PLAY_ACTION_HIT) {
        return;
    }

    if(!canHit(game)) {
        return;
    }

    addCard(game, hand);

    if(Table_handValue(*hand) > 21) {
        game->playRound.playerHands[game->playRound.activePlayerHand].result =
            ROUND_RESULT_PLAYER_BUST;
        finishActiveHand(game);
    }
}

void playStand(Runtime_Game *game) {
    if(decideActivePlayerAction(game, PLAY_ACTION_STAND)
        != PLAY_ACTION_STAND) {
        return;
    }

    if(game->playRound.phase != ROUND_PLAYER_TURN) {
        return;
    }

    finishActiveHand(game);
}

void playDouble(Runtime_Game *game) {
    Table_Hand *hand = activePlayerHand(game);

    if(decideActivePlayerAction(game, PLAY_ACTION_DOUBLE)
        != PLAY_ACTION_DOUBLE) {
        return;
    }

    if(!canDouble(game) || !hand) {
        return;
    }

    game->playRound.playerHands[game->playRound.activePlayerHand].doubledDown =
        true;
    addCard(game, hand);

    if(Table_handValue(*hand) > 21) {
        game->playRound.playerHands[game->playRound.activePlayerHand].result =
            ROUND_RESULT_PLAYER_BUST;
    }

    finishActiveHand(game);
}

void playSplit(Runtime_Game *game) {
    PlayRound *round = &game->playRound;
    Table_Hand *firstHand = activePlayerHand(game);
    const int insertIndex = round->activePlayerHand + 1;
    PlayHand *secondPlayHand;
    Table_Hand *secondHand;
    Table_Card movedCard;

    if(decideActivePlayerAction(game, PLAY_ACTION_SPLIT)
        != PLAY_ACTION_SPLIT) {
        return;
    }

    if(!canSplit(game) || !firstHand) {
        return;
    }

    movedCard = firstHand->cards[1];

    for(int index = round->playerHandCount; index > insertIndex; index -= 1) {
        round->playerHands[index] = round->playerHands[index - 1];
    }

    secondPlayHand = &round->playerHands[insertIndex];
    secondHand = &secondPlayHand->cards;
    clearHand(secondHand);
    secondHand->cards[0] = movedCard;
    secondHand->count = 1;
    firstHand->count = 1;
    round->playerHandCount += 1;
    round->playerHands[round->activePlayerHand].result = ROUND_RESULT_NONE;
    secondPlayHand->result = ROUND_RESULT_NONE;
    round->playerHands[round->activePlayerHand].doubledDown = false;
    secondPlayHand->doubledDown = false;
    round->playerHands[round->activePlayerHand].splitAces =
        firstHand->cards[0].rank == TABLE_RANK_ACE;
    secondPlayHand->splitAces = movedCard.rank == TABLE_RANK_ACE;
    secondPlayHand->decision = userDecision();

    addCard(game, firstHand);
    addCard(game, secondHand);

    if(activeHandIsSplitAces(game) && !game->playSettings.allowHitSplitAces
        && !canSplit(game)) {
        finishActiveHand(game);
    }
}

void playSurrender(Runtime_Game *game) {
    PlayRound *round = &game->playRound;

    if(decideActivePlayerAction(game, PLAY_ACTION_SURRENDER)
        != PLAY_ACTION_SURRENDER) {
        return;
    }

    if(!canSurrender(game)) {
        return;
    }

    round->playerHands[round->activePlayerHand].result =
        ROUND_RESULT_SURRENDER;
    finishActiveHand(game);
}

Table_Hand *activePlayerHand(Runtime_Game *game) {
    PlayRound *round = &game->playRound;

    if(round->activePlayerHand < 0
        || round->activePlayerHand >= round->playerHandCount) {
        return 0;
    }

    return &round->playerHands[round->activePlayerHand].cards;
}

RoundResult playerHandResult(Runtime_Game *game, int handIndex) {
    PlayRound *round = &game->playRound;

    if(handIndex < 0 || handIndex >= round->playerHandCount) {
        return ROUND_RESULT_NONE;
    }

    return round->playerHands[handIndex].result;
}

bool canDouble(Runtime_Game *game) {
    PlayRound *round = &game->playRound;
    Table_Hand *hand = activePlayerHand(game);

    if(round->phase != ROUND_PLAYER_TURN || !hand || hand->count != 2
        || !game->playSettings.allowDoubleDown || activeHandIsSplitAces(game)
        || !doubleRuleAllows(game, *hand)) {
        return false;
    }

    return round->playerHandCount == 1
        || game->playSettings.allowDoubleAfterSplit;
}

bool canHit(Runtime_Game *game) {
    return game->playRound.phase == ROUND_PLAYER_TURN && activePlayerHand(game)
        && canActOnSplitAces(game);
}

bool canSplit(Runtime_Game *game) {
    PlayHand *playHand =
        &game->playRound.playerHands[game->playRound.activePlayerHand];
    Table_Hand *hand = activePlayerHand(game);

    if(game->playRound.phase != ROUND_PLAYER_TURN
        || !playHand->decision.allowSplit || !game->playSettings.allowSplit
        || !splitLimitAllows(game) || !activeHandHasPair(game)) {
        return false;
    }

    return !handIsAcePair(*hand) || !activeHandIsSplitAces(game)
        || game->playSettings.allowResplitAces;
}

bool canSurrender(Runtime_Game *game) {
    Table_Hand *hand = activePlayerHand(game);
    PlayHand *playHand =
        &game->playRound.playerHands[game->playRound.activePlayerHand];

    return game->playRound.phase == ROUND_PLAYER_TURN
        && playHand->decision.allowSurrender
        && game->playSettings.allowSurrender && hand && hand->count == 2
        && game->playRound.playerHandCount == 1;
}
