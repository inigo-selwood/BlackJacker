#ifndef BLACKJACKER_MODES_PLAY_GAME_H
#define BLACKJACKER_MODES_PLAY_GAME_H

#include <stdbool.h>

#include "runtime/runtime.h"

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

#endif
