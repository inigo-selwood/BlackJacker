#ifndef BLACKJACKER_MODES_PLAY_MODE_H
#define BLACKJACKER_MODES_PLAY_MODE_H

#include <stdbool.h>

#include "graphics/graphics.h"
#include "table/table.h"

typedef struct Runtime_Game Runtime_Game;
typedef struct Runtime_ModeEvent Runtime_ModeEvent;

/*******************************************************************************
 * Settings
 ******************************************************************************/

enum {
    MAX_PLAYER_HANDS = 4,
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

/*******************************************************************************
 * Round State
 ******************************************************************************/

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
    PlayHand playerHands[MAX_PLAYER_HANDS];
    int roundNumber;
    int playerHandCount;
    int activePlayerHand;
    RoundPhase phase;
} PlayRound;

/*******************************************************************************
 * Mode Lifecycle
 ******************************************************************************/

/** Handles lifecycle events for the play mode. */
void playCallback(Runtime_Game *game, Runtime_ModeEvent event);

/*******************************************************************************
 * Game Actions
 ******************************************************************************/

/** Initializes play table state and deals the first round. */
void initPlayRound(Runtime_Game *game);

/** Clears the active hands and deals a new round. */
void startNextPlayRound(Runtime_Game *game);

/** Adds one card to the player hand and resolves busts. */
void playHit(Runtime_Game *game);

/** Ends the player turn and lets the dealer resolve the hand. */
void playStand(Runtime_Game *game);

/** Doubles the hand, takes one card, then resolves the dealer hand. */
void playDouble(Runtime_Game *game);

/** Splits a pair into two player hands when rules and cards allow it. */
void playSplit(Runtime_Game *game);

/** Surrenders the active hand when allowed. */
void playSurrender(Runtime_Game *game);

/** Returns the hand currently receiving player actions. */
Table_Hand *activePlayerHand(Runtime_Game *game);

/** Returns the completed result for a player hand. */
RoundResult playerHandResult(Runtime_Game *game, int handIndex);

/** Returns whether the active hand can currently be doubled. */
bool canDouble(Runtime_Game *game);

/** Returns whether the active hand can currently be hit. */
bool canHit(Runtime_Game *game);

/** Returns whether the active hand can currently be split. */
bool canSplit(Runtime_Game *game);

/** Returns whether the active hand can currently be surrendered. */
bool canSurrender(Runtime_Game *game);

/*******************************************************************************
 * Controls
 ******************************************************************************/

/** Builds and positions controls for the current play state. */
Graphics_ControlGroup playControls(Runtime_Game *game, Graphics_Box box);

/*******************************************************************************
 * Rendering
 ******************************************************************************/

/** Draws the full play table for the current round. */
void drawPlay(Runtime_Game *game);

#endif
