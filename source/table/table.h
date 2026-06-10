#ifndef BLACKJACKER_TABLE_H
#define BLACKJACKER_TABLE_H

#include <stdbool.h>

/*******************************************************************************
 * Cards
 ******************************************************************************/

/** Table_Card rank values from ace through king. */
typedef enum {
    TABLE_RANK_ACE,
    TABLE_RANK_TWO,
    TABLE_RANK_THREE,
    TABLE_RANK_FOUR,
    TABLE_RANK_FIVE,
    TABLE_RANK_SIX,
    TABLE_RANK_SEVEN,
    TABLE_RANK_EIGHT,
    TABLE_RANK_NINE,
    TABLE_RANK_TEN,
    TABLE_RANK_JACK,
    TABLE_RANK_QUEEN,
    TABLE_RANK_KING
} Table_Rank;

/** Table_Card suit values. */
typedef enum {
    TABLE_SUIT_SPADES,
    TABLE_SUIT_HEARTS,
    TABLE_SUIT_DIAMONDS,
    TABLE_SUIT_CLUBS
} Table_Suit;

/** Immutable card identity used by game and rendering code. */
typedef struct {
    Table_Rank rank;
    Table_Suit suit;
} Table_Card;

/** Returns the compact display label for a rank. */
const char *Table_rankLabel(Table_Rank rank);

/** Returns the one-letter display label for a suit. */
const char *Table_suitLetter(Table_Suit suit);

/** Returns the UTF-8 glyph for a suit. */
const char *Table_suitSymbol(Table_Suit suit);

/*******************************************************************************
 * Hands
 ******************************************************************************/

enum { TABLE_MAX_HAND_CARDS = 12 };

/** Blackjack hand plus cached card count. */
typedef struct {
    Table_Card cards[TABLE_MAX_HAND_CARDS];
    int count;
} Table_Hand;

/** Returns the best blackjack value for a hand. */
int Table_handValue(Table_Hand hand);

/** Returns true when the hand contains an ace counted as eleven. */
bool Table_handIsSoft(Table_Hand hand);

/*******************************************************************************
 * Shoes
 ******************************************************************************/

enum {
    TABLE_CARDS_PER_DECK = 52,
    TABLE_MAX_SHOE_CARDS = 416,
};

/** Dealt shoe following the configured deck count and cut-card position. */
typedef struct {
    Table_Card cards[TABLE_MAX_SHOE_CARDS];
    int cardCount;
    int nextIndex;
    int cutIndex;
    int deckCount;
    int cutPercent;
    unsigned int seed;
} Table_Shoe;

/** Builds and shuffles a shoe using the current play settings. */
void Table_initShoe(Table_Shoe *shoe, int deckCount, int cutPercent);

/** Rebuilds and shuffles the shoe while preserving its random sequence. */
void Table_shuffleShoe(Table_Shoe *shoe, int deckCount, int cutPercent);

/** Draws a card, reshuffling first if the cut card has been reached. */
Table_Card Table_drawShoeCard(Table_Shoe *shoe, int deckCount, int cutPercent);

/** Returns how many cards remain before the cut card is reached. */
int Table_shoeCardsUntilCut(Table_Shoe shoe);

#endif
