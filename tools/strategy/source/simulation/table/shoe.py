"""Shoe primitives mirroring the C table module."""

from dataclasses import dataclass, field

from simulation.table.card import Card, Rank, Suit

CARDS_PER_DECK = 52
INITIAL_SEED = 0xC0FFEE


@dataclass
class Shoe:
    """Dealt shoe following configured deck count and cut-card position."""

    cards: list[Card] = field(default_factory=list)
    next_index: int = 0
    cut_index: int = 0
    deck_count: int = 0
    cut_percent: int = 0
    seed: int = INITIAL_SEED


def clamp(value: int, minimum: int, maximum: int) -> int:
    """Clamp an integer to a range."""
    if value < minimum:
        return minimum

    if value > maximum:
        return maximum

    return value


def add_deck(shoe: Shoe) -> None:
    """Add one ordered deck to a shoe."""
    for suit in Suit:
        for rank in Rank:
            shoe.cards.append(Card(rank=rank, suit=suit))


def shuffle_cards(shoe: Shoe) -> None:
    """Shuffle cards using the same deterministic generator as the C app."""
    for index in range(len(shoe.cards) - 1, 0, -1):
        # Match the C app's simple linear congruential generator.
        shoe.seed = (shoe.seed * 1103515245 + 12345) & 0xFFFFFFFF
        swap_index = shoe.seed % (index + 1)
        shoe.cards[index], shoe.cards[swap_index] = (
            shoe.cards[swap_index],
            shoe.cards[index],
        )


def shuffle(shoe: Shoe, deck_count: int, cut_percent: int) -> None:
    """Rebuild and shuffle a shoe while preserving its random sequence."""
    deck_count = clamp(deck_count, 1, 8)
    cut_percent = clamp(cut_percent, 1, 100)

    shoe.cards.clear()
    shoe.next_index = 0
    shoe.deck_count = deck_count
    shoe.cut_percent = cut_percent

    for _ in range(deck_count):
        add_deck(shoe)

    shoe.cut_index = len(shoe.cards) * cut_percent // 100

    if shoe.cut_index <= 0:
        shoe.cut_index = 1

    shuffle_cards(shoe)


def create(deck_count: int, cut_percent: int) -> Shoe:
    """Build and shuffle a shoe using the current play settings."""
    shoe = Shoe()
    shuffle(shoe, deck_count, cut_percent)
    return shoe


def draw(shoe: Shoe, deck_count: int, cut_percent: int) -> Card:
    """Draw a card, reshuffling first if the cut card has been reached."""
    deck_count = clamp(deck_count, 1, 8)
    cut_percent = clamp(cut_percent, 1, 100)

    if (
        not shoe.cards
        or shoe.next_index >= shoe.cut_index
        or shoe.deck_count != deck_count
        or shoe.cut_percent != cut_percent
    ):
        shuffle(shoe, deck_count, cut_percent)

    card = shoe.cards[shoe.next_index]
    shoe.next_index += 1
    return card


def cards_until_cut(shoe: Shoe) -> int:
    """Return how many cards remain before the cut card is reached."""
    return max(shoe.cut_index - shoe.next_index, 0)
