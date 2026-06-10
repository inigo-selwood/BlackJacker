"""Card primitives mirroring the C table module."""

from dataclasses import dataclass
from enum import IntEnum


class Rank(IntEnum):
    """Card rank from ace through king."""

    ACE = 1
    TWO = 2
    THREE = 3
    FOUR = 4
    FIVE = 5
    SIX = 6
    SEVEN = 7
    EIGHT = 8
    NINE = 9
    TEN = 10
    JACK = 11
    QUEEN = 12
    KING = 13


class Suit(IntEnum):
    """Card suit."""

    SPADES = 0
    HEARTS = 1
    DIAMONDS = 2
    CLUBS = 3


@dataclass(frozen=True)
class Card:
    """Immutable card identity."""

    rank: Rank
    suit: Suit


def rank_label(rank: Rank) -> str:
    """Return the compact display label for a rank."""
    if rank == Rank.ACE:
        return "A"

    if rank == Rank.JACK:
        return "J"

    if rank == Rank.QUEEN:
        return "Q"

    if rank == Rank.KING:
        return "K"

    return str(int(rank))


def suit_letter(suit: Suit) -> str:
    """Return the one-letter display label for a suit."""
    return {
        Suit.SPADES: "S",
        Suit.HEARTS: "H",
        Suit.DIAMONDS: "D",
        Suit.CLUBS: "C",
    }[suit]


def suit_symbol(suit: Suit) -> str:
    """Return the UTF-8 glyph for a suit."""
    return {
        Suit.SPADES: "♠",
        Suit.HEARTS: "♥",
        Suit.DIAMONDS: "♦",
        Suit.CLUBS: "♣",
    }[suit]


def rank_value(rank: Rank) -> int:
    """Return the blackjack strategy value for a rank."""
    if rank == Rank.ACE:
        return 11

    if rank >= Rank.TEN:
        return 10

    return int(rank)
