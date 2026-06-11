"""Dealer outcome simulation helpers."""

from functools import cache

from settings import DealerSoft17Rule
from simulation.table.card import Card, Rank, rank_value

RANK_PROBABILITIES = {
    Rank.ACE: 1.0 / 13.0,
    Rank.TWO: 1.0 / 13.0,
    Rank.THREE: 1.0 / 13.0,
    Rank.FOUR: 1.0 / 13.0,
    Rank.FIVE: 1.0 / 13.0,
    Rank.SIX: 1.0 / 13.0,
    Rank.SEVEN: 1.0 / 13.0,
    Rank.EIGHT: 1.0 / 13.0,
    Rank.NINE: 1.0 / 13.0,
    Rank.TEN: 4.0 / 13.0,
}


def add_card(total: int, soft_aces: int, rank: Rank) -> tuple[int, int]:
    """Add one card to a dealer hand total."""
    total += rank_value(rank)

    if rank == Rank.ACE:
        soft_aces += 1

    while total > 21 and soft_aces > 0:
        total -= 10
        soft_aces -= 1

    return total, soft_aces


def should_stand(
    total: int,
    soft_aces: int,
    soft_17_rule: DealerSoft17Rule,
) -> bool:
    """Return whether the dealer stands on the current total."""
    if total > 17:
        return True

    if total < 17:
        return False

    return soft_aces == 0 or soft_17_rule == DealerSoft17Rule.STAND


@cache
def draw_outcomes(
    total: int,
    soft_aces: int,
    soft_17_rule: DealerSoft17Rule,
) -> dict[str, float]:
    """Return dealer outcome probabilities from a current total."""
    if should_stand(total, soft_aces, soft_17_rule):
        return {"bust" if total > 21 else str(total): 1.0}

    outcomes: dict[str, float] = {}

    for rank, probability in RANK_PROBABILITIES.items():
        next_total, next_soft_aces = add_card(total, soft_aces, rank)
        next_outcomes = draw_outcomes(
            next_total,
            next_soft_aces,
            soft_17_rule,
        )

        for outcome, outcome_probability in next_outcomes.items():
            outcomes[outcome] = (
                outcomes.get(outcome, 0.0) + outcome_probability * probability
            )

    return outcomes


def outcomes(
    upcard: Card,
    soft_17_rule: DealerSoft17Rule,
) -> dict[str, float]:
    """Return final dealer outcome probabilities for an upcard."""
    value = rank_value(upcard.rank)
    soft_aces = 1 if upcard.rank == Rank.ACE else 0
    return draw_outcomes(value, soft_aces, soft_17_rule)
