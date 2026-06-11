"""Finite shoe composition helpers."""

from dataclasses import dataclass

from simulation.table.card import Rank

STRATEGY_RANKS = (
    Rank.ACE,
    Rank.TWO,
    Rank.THREE,
    Rank.FOUR,
    Rank.FIVE,
    Rank.SIX,
    Rank.SEVEN,
    Rank.EIGHT,
    Rank.NINE,
    Rank.TEN,
)


@dataclass(frozen=True)
class Composition:
    """Remaining rank counts in a blackjack shoe."""

    counts: tuple[int, ...]


def fresh(deck_count: int) -> Composition:
    """Return a fresh shoe composition for a deck count."""
    return Composition((4 * deck_count,) * 9 + (16 * deck_count,))


def remove(composition: Composition, rank: Rank) -> Composition:
    """Return composition after removing one rank."""
    index = _rank_index(rank)
    counts = list(composition.counts)

    if counts[index] <= 0:
        return composition

    counts[index] -= 1
    return Composition(tuple(counts))


def draw_options(
    composition: Composition,
) -> tuple[tuple[Rank, float, Composition], ...]:
    """Return possible drawn ranks, probabilities, and next compositions."""
    total = sum(composition.counts)

    if total <= 0:
        return ()

    options: list[tuple[Rank, float, Composition]] = []

    for rank, count in zip(STRATEGY_RANKS, composition.counts):
        if count <= 0:
            continue

        options.append((rank, count / total, remove(composition, rank)))

    return tuple(options)


def _rank_index(rank: Rank) -> int:
    """Return composition index for a rank."""
    return 9 if rank >= Rank.TEN else int(rank) - 1
