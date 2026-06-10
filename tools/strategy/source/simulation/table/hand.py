"""Hand primitives mirroring the C table module."""

from dataclasses import dataclass, field

from simulation.table.card import Card, Rank, rank_value


@dataclass
class Hand:
    """Blackjack hand."""

    cards: list[Card] = field(default_factory=list)


def value(hand: Hand) -> int:
    """Return the best blackjack value for a hand."""
    total = 0
    ace_count = 0

    for card in hand.cards:
        total += rank_value(card.rank)

        if card.rank == Rank.ACE:
            ace_count += 1

    while total > 21 and ace_count > 0:
        total -= 10
        ace_count -= 1

    return total


def is_soft(hand: Hand) -> bool:
    """Return true when the hand contains an ace counted as eleven."""
    total = 0
    ace_count = 0

    for card in hand.cards:
        total += rank_value(card.rank)

        if card.rank == Rank.ACE:
            ace_count += 1

    while total > 21 and ace_count > 0:
        total -= 10
        ace_count -= 1

    return ace_count > 0
