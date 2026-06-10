#ifndef BLACKJACKER_MODES_PLAY_STATE_H
#define BLACKJACKER_MODES_PLAY_STATE_H

#include <stdbool.h>

#include "table/table.h"

enum {
    MAX_PLAYER_HANDS = 4,
    MAX_OTHER_PLAYER_HANDS = 2,
    PLAYER_TABLE_SEAT_INDEX = 1,
};

/** Dealer behavior for soft 17. */
typedef enum { DEALER_HITS_SOFT_17, DEALER_STANDS_SOFT_17 } DealerSoft17Rule;

/** Blackjack payout rule. */
typedef enum { BLACKJACK_PAYS_3_TO_2, BLACKJACK_PAYS_6_TO_5 } BlackjackPayout;

/** Player double-down total restrictions. */
typedef enum {
    DOUBLE_ANY_TWO,
    DOUBLE_9_10_11,
    DOUBLE_10_11,
} DoubleRule;

/** Configurable play/gameplay settings. */
typedef struct {
    int deckCount;
    int cutPercent;
    bool allowDoubleDown;
    bool allowDoubleAfterSplit;
    DoubleRule doubleRule;
    bool allowSplit;
    bool allowResplit;
    bool allowHitSplitAces;
    bool allowResplitAces;
    bool allowSurrender;
    bool useNoHoleCardRule;
    bool showTrueCount;
    DealerSoft17Rule dealerSoft17Rule;
    BlackjackPayout blackjackPayout;
} PlaySettings;

/** Current lifecycle step for the active play round. */
typedef enum {
    ROUND_PLAYER_TURN,
    ROUND_COMPLETE,
} RoundPhase;

/** Outcome once a play round has resolved. */
typedef enum {
    ROUND_RESULT_NONE,
    ROUND_RESULT_PLAYER_BUST,
    ROUND_RESULT_DEALER_BUST,
    ROUND_RESULT_PLAYER_WIN,
    ROUND_RESULT_DEALER_WIN,
    ROUND_RESULT_PUSH,
    ROUND_RESULT_SURRENDER,
} RoundResult;

/** How a hand obtains its next action. */
typedef enum {
    PLAY_DECISION_USER,
    PLAY_DECISION_STRATEGY,
} PlayDecisionSource;

/** Optional player action provided by a decision source. */
typedef enum {
    PLAY_ACTION_NONE,
    PLAY_ACTION_HIT,
    PLAY_ACTION_STAND,
    PLAY_ACTION_DOUBLE,
    PLAY_ACTION_SPLIT,
    PLAY_ACTION_SURRENDER,
} PlayAction;

/** Decision configuration for a hand at the table. */
typedef struct {
    PlayDecisionSource source;
    bool allowSurrender;
    bool allowSplit;
} PlayDecision;

/** A blackjack hand plus action/result metadata. */
typedef struct {
    Table_Hand cards;
    RoundResult result;
    PlayDecision decision;
    bool doubledDown;
    bool splitAces;
} PlayHand;

/** Mutable state for the blackjack play table. */
typedef struct {
    Table_Hand dealerHand;
    PlayHand otherPlayerHands[MAX_OTHER_PLAYER_HANDS];
    PlayHand playerHands[MAX_PLAYER_HANDS];
    int roundNumber;
    int otherPlayerHandCount;
    int playerHandCount;
    int activePlayerHand;
    RoundPhase phase;
} PlayRound;

#endif
