#include "table/table.h"

static int clampedInt(int value, int min, int max) {
    if(value < min) {
        return min;
    }

    if(value > max) {
        return max;
    }

    return value;
}

static unsigned int nextRandom(Table_Shoe *shoe) {
    shoe->seed = shoe->seed * 1103515245u + 12345u;
    return shoe->seed;
}

static void addDeck(Table_Shoe *shoe) {
    for(int suit = TABLE_SUIT_SPADES; suit <= TABLE_SUIT_CLUBS; suit += 1) {
        for(int rank = TABLE_RANK_ACE; rank <= TABLE_RANK_KING; rank += 1) {
            shoe->cards[shoe->cardCount] = (Table_Card){
                (Table_Rank)rank,
                (Table_Suit)suit,
            };
            shoe->cardCount += 1;
        }
    }
}

static void shuffleCards(Table_Shoe *shoe) {
    for(int index = shoe->cardCount - 1; index > 0; index -= 1) {
        const int swapIndex =
            (int)(nextRandom(shoe) % (unsigned int)(index + 1));
        const Table_Card card = shoe->cards[index];

        shoe->cards[index] = shoe->cards[swapIndex];
        shoe->cards[swapIndex] = card;
    }
}

void Table_initShoe(Table_Shoe *shoe, int deckCount, int cutPercent) {
    *shoe = (Table_Shoe){
        .seed = 0xC0FFEEu,
    };
    Table_shuffleShoe(shoe, deckCount, cutPercent);
}

void Table_shuffleShoe(Table_Shoe *shoe, int deckCount, int cutPercent) {
    deckCount = clampedInt(deckCount, 1, 8);
    cutPercent = clampedInt(cutPercent, 1, 100);

    shoe->cardCount = 0;
    shoe->nextIndex = 0;
    shoe->deckCount = deckCount;
    shoe->cutPercent = cutPercent;

    for(int deck = 0; deck < deckCount; deck += 1) {
        addDeck(shoe);
    }

    shoe->cutIndex = shoe->cardCount * cutPercent / 100;

    if(shoe->cutIndex <= 0) {
        shoe->cutIndex = 1;
    }

    shuffleCards(shoe);
}

Table_Card
Table_drawShoeCard(Table_Shoe *shoe, int deckCount, int cutPercent) {
    deckCount = clampedInt(deckCount, 1, 8);
    cutPercent = clampedInt(cutPercent, 1, 100);

    if(shoe->cardCount <= 0 || shoe->nextIndex >= shoe->cutIndex
        || shoe->deckCount != deckCount || shoe->cutPercent != cutPercent) {
        Table_shuffleShoe(shoe, deckCount, cutPercent);
    }

    const Table_Card card = shoe->cards[shoe->nextIndex];

    shoe->nextIndex += 1;

    return card;
}

int Table_shoeCardsUntilCut(Table_Shoe shoe) {
    const int remaining = shoe.cutIndex - shoe.nextIndex;

    return remaining < 0 ? 0 : remaining;
}
