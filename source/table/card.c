#include "table/table.h"

const char *Table_rankLabel(Table_Rank rank) {
    static const char *labels[] =
        {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};

    if(rank < TABLE_RANK_ACE || rank > TABLE_RANK_KING) {
        return "?";
    }

    return labels[rank];
}

const char *Table_suitLetter(Table_Suit suit) {
    static const char *letters[] = {"S", "H", "D", "C"};

    if(suit < TABLE_SUIT_SPADES || suit > TABLE_SUIT_CLUBS) {
        return "?";
    }

    return letters[suit];
}

const char *Table_suitSymbol(Table_Suit suit) {
    static const char *symbols[] = {"♠", "♥", "♦", "♣"};

    if(suit < TABLE_SUIT_SPADES || suit > TABLE_SUIT_CLUBS) {
        return "?";
    }

    return symbols[suit];
}
