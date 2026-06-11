"""Expected value calculation helpers."""

from functools import cache

from settings import PlaySettings
from simulation.table import dealer
from simulation.table.card import (
    Card,
    RANK_PROBABILITIES,
    Rank,
    Suit,
    rank_value,
)

SURRENDER_EV = -0.5
MAX_SPLIT_HANDS = 4


def _add_card(total: int, soft_aces: int, rank: Rank) -> tuple[int, int]:
    """Return a player total after drawing one rank."""
    total += rank_value(rank)

    if rank == Rank.ACE:
        soft_aces += 1

    while total > 21 and soft_aces > 0:
        total -= 10
        soft_aces -= 1

    return total, soft_aces


@cache
def stand(
    player_total: int,
    dealer_rank: Rank,
    settings: PlaySettings,
) -> float:
    """Return EV for standing on a player total."""
    if player_total > 21:
        return -1.0

    expected_value = 0.0
    dealer_upcard = Card(rank=dealer_rank, suit=Suit.SPADES)
    outcomes = dealer.outcomes(dealer_upcard, settings.dealer_soft_17_rule)

    for outcome, probability in outcomes.items():
        if outcome == "bust":
            expected_value += probability
            continue

        dealer_total = int(outcome)

        if player_total > dealer_total:
            expected_value += probability
        elif player_total < dealer_total:
            expected_value -= probability

    return expected_value


@cache
def hit(
    player_total: int,
    soft_aces: int,
    dealer_rank: Rank,
    settings: PlaySettings,
) -> float:
    """Return EV for hitting once, then playing hit/stand optimally."""
    expected_value = 0.0

    for rank, probability in RANK_PROBABILITIES.items():
        next_total, next_soft_aces = _add_card(player_total, soft_aces, rank)

        if next_total > 21:
            outcome_ev = -1.0
        else:
            outcome_ev = max(
                stand(next_total, dealer_rank, settings),
                hit(next_total, next_soft_aces, dealer_rank, settings),
            )

        expected_value += outcome_ev * probability

    return expected_value


@cache
def double(
    player_total: int,
    soft_aces: int,
    dealer_rank: Rank,
    settings: PlaySettings,
) -> float:
    """Return EV for doubling down."""
    expected_value = 0.0

    for rank, probability in RANK_PROBABILITIES.items():
        next_total, _ = _add_card(player_total, soft_aces, rank)
        expected_value += (
            stand(next_total, dealer_rank, settings) * probability
        )

    return expected_value * 2.0


def _can_resplit(
    pair_rank: Rank,
    hand_count: int,
    settings: PlaySettings,
) -> bool:
    """Return whether a split hand can be split again."""
    if hand_count >= MAX_SPLIT_HANDS or not settings.allow_resplit:
        return False

    return pair_rank != Rank.ACE or settings.allow_resplit_aces


@cache
def split(
    pair_total: int,
    dealer_rank: Rank,
    settings: PlaySettings,
    hand_count: int = 2,
) -> float:
    """Return EV for splitting a pair, including allowed resplits."""
    pair_rank = Rank.ACE if pair_total == 22 else Rank(pair_total // 2)
    first_total, first_soft_aces = _add_card(0, 0, pair_rank)
    expected_hand_value = 0.0

    for rank, probability in RANK_PROBABILITIES.items():
        total, soft_aces = _add_card(first_total, first_soft_aces, rank)

        if rank == pair_rank and _can_resplit(pair_rank, hand_count, settings):
            hand_value = split(
                pair_total,
                dealer_rank,
                settings,
                hand_count + 1,
            )
        elif pair_rank == Rank.ACE and not settings.allow_hit_split_aces:
            hand_value = stand(total, dealer_rank, settings)
        else:
            hand_value = max(
                stand(total, dealer_rank, settings),
                hit(total, soft_aces, dealer_rank, settings),
            )

            if settings.allow_double_after_split:
                hand_value = max(
                    hand_value,
                    double(total, soft_aces, dealer_rank, settings),
                )

        expected_hand_value += hand_value * probability

    return expected_hand_value * 2.0
