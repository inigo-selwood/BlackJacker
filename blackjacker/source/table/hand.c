#include "table/table.h"

static int rankValue(Table_Rank rank) {
    if(rank == TABLE_RANK_ACE) {
        return 11;
    }

    if(rank >= TABLE_RANK_TEN) {
        return 10;
    }

    return (int)rank + 1;
}

int Table_handValue(Table_Hand hand) {
    int value = 0;
    int aceCount = 0;

    for(int index = 0; index < hand.count; index += 1) {
        value += rankValue(hand.cards[index].rank);

        if(hand.cards[index].rank == TABLE_RANK_ACE) {
            aceCount += 1;
        }
    }

    while(value > 21 && aceCount > 0) {
        value -= 10;
        aceCount -= 1;
    }

    return value;
}

bool Table_handIsSoft(Table_Hand hand) {
    int value = 0;
    int aceCount = 0;

    for(int index = 0; index < hand.count; index += 1) {
        value += rankValue(hand.cards[index].rank);

        if(hand.cards[index].rank == TABLE_RANK_ACE) {
            aceCount += 1;
        }
    }

    while(value > 21 && aceCount > 0) {
        value -= 10;
        aceCount -= 1;
    }

    return aceCount > 0;
}
