"""First-pass strategy row generation."""

from dataclasses import dataclass
from enum import StrEnum

import orm
from schema import ExpectedValue
from settings import DoubleRule, PlaySettings
from simulation import ev
from simulation.table.card import Rank


class HandKind(StrEnum):
    """Player hand category used for strategy lookup."""

    HARD = "hard"
    SOFT = "soft"
    PAIR = "pair"


class Action(StrEnum):
    """Player actions with expected values."""

    HIT = "hit"
    STAND = "stand"
    DOUBLE = "double"
    SPLIT = "split"
    SURRENDER = "surrender"


_DEALER_RANKS = [
    Rank.TWO,
    Rank.THREE,
    Rank.FOUR,
    Rank.FIVE,
    Rank.SIX,
    Rank.SEVEN,
    Rank.EIGHT,
    Rank.NINE,
    Rank.TEN,
    Rank.ACE,
]


@dataclass(frozen=True)
class StrategyContext:
    """Strategy decision context for one lookup row."""

    hand_kind: HandKind
    player_total: int
    dealer_rank: Rank


def _context_state(context: StrategyContext) -> tuple[int, int]:
    """Return blackjack total and soft ace count for EV calculation."""
    if context.hand_kind == HandKind.SOFT:
        return context.player_total, 1

    if context.hand_kind == HandKind.PAIR and context.player_total == 22:
        return 12, 1

    return context.player_total, 0


def _double_available(settings: PlaySettings, player_total: int) -> bool:
    """Return whether double is legal for a total."""
    if not settings.allow_double_down:
        return False

    if settings.double_rule == DoubleRule.NINE_TEN_ELEVEN:
        return 9 <= player_total <= 11

    if settings.double_rule == DoubleRule.TEN_ELEVEN:
        return player_total in (10, 11)

    return True


def _action_available(
    settings: PlaySettings,
    context: StrategyContext,
    action: Action,
) -> bool:
    """Return whether an action is legal in a strategy context."""
    player_total, _ = _context_state(context)

    if action in (Action.STAND, Action.HIT):
        return True

    if action == Action.DOUBLE:
        return _double_available(settings, player_total)

    if action == Action.SURRENDER:
        return settings.allow_surrender

    return context.hand_kind == HandKind.PAIR and settings.allow_split


def _action_ev(
    settings: PlaySettings,
    context: StrategyContext,
    action: Action,
) -> float:
    """Return EV for one action in a strategy context."""
    player_total, soft_aces = _context_state(context)

    if action == Action.STAND:
        return ev.stand(player_total, context.dealer_rank, settings)

    if action == Action.HIT:
        return ev.hit(player_total, soft_aces, context.dealer_rank, settings)

    if action == Action.DOUBLE:
        return ev.double(
            player_total, soft_aces, context.dealer_rank, settings
        )

    if action == Action.SURRENDER:
        return ev.SURRENDER_EV

    return ev.split(context.player_total, context.dealer_rank, settings)


def _contexts() -> list[StrategyContext]:
    """Return the first-pass strategy contexts to generate."""
    result: list[StrategyContext] = []

    for total in range(5, 22):
        for rank in _DEALER_RANKS:
            result.append(StrategyContext(HandKind.HARD, total, rank))

    for total in range(13, 22):
        for rank in _DEALER_RANKS:
            result.append(StrategyContext(HandKind.SOFT, total, rank))

    for total in range(4, 23, 2):
        for rank in _DEALER_RANKS:
            result.append(StrategyContext(HandKind.PAIR, total, rank))

    return result


def expected_values(settings: PlaySettings) -> list[ExpectedValue]:
    """Generate first-pass expected value rows."""
    rows: list[ExpectedValue] = []

    for context in _contexts():
        for action in Action:
            available = _action_available(settings, context, action)
            rows.append(
                ExpectedValue(
                    hand_kind=orm.to_orm(context.hand_kind),
                    player_total=context.player_total,
                    dealer_upcard=orm.to_orm(context.dealer_rank),
                    action=orm.to_orm(action),
                    available=available,
                    ev=(
                        _action_ev(settings, context, action)
                        if available
                        else 0.0
                    ),
                )
            )

    return rows
