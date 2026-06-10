"""First-pass strategy row generation."""

from dataclasses import dataclass
from enum import StrEnum

from schema import ExpectedValue
from settings import DealerSoft17Rule, PlaySettings


class StrategyEnum(StrEnum):
    """String enum with storage helpers."""

    def serialize(self) -> str:
        """Return the value stored in SQLite."""
        return self.value

    @classmethod
    def values(cls) -> list[str]:
        """Return all serialized enum values."""
        return [item.serialize() for item in cls]


class HandKind(StrategyEnum):
    """Player hand category used for strategy lookup."""

    HARD = "hard"
    SOFT = "soft"
    PAIR = "pair"


class DealerUpcard(StrategyEnum):
    """Dealer upcard values used for strategy lookup."""

    TWO = "2"
    THREE = "3"
    FOUR = "4"
    FIVE = "5"
    SIX = "6"
    SEVEN = "7"
    EIGHT = "8"
    NINE = "9"
    TEN = "10"
    ACE = "ace"


class Action(StrategyEnum):
    """Player actions with expected values."""

    HIT = "hit"
    STAND = "stand"
    DOUBLE = "double"
    SPLIT = "split"
    SURRENDER = "surrender"

@dataclass(frozen=True)
class StrategyContext:
    """Strategy decision context for one lookup row."""

    hand_kind: HandKind
    player_total: int
    dealer_upcard: DealerUpcard


def dealer_value(upcard: DealerUpcard) -> int:
    """Return the numeric strategy value for a dealer upcard."""
    if upcard == DealerUpcard.ACE:
        return 11
    return int(upcard.serialize())


def double_or_hit(settings: PlaySettings, total: int) -> Action:
    """Return double if the configured double rule permits it."""
    if not settings.allow_double_down:
        return Action.HIT

    if settings.double_rule == "9-10-11":
        return Action.DOUBLE if 9 <= total <= 11 else Action.HIT

    if settings.double_rule == "10-11":
        return Action.DOUBLE if total in (10, 11) else Action.HIT

    return Action.DOUBLE


def surrender_or_hit(settings: PlaySettings) -> Action:
    """Return surrender if available, otherwise hit."""
    return Action.SURRENDER if settings.allow_surrender else Action.HIT


def split_or_fallback(settings: PlaySettings, fallback: Action) -> Action:
    """Return split if available, otherwise the fallback action."""
    return Action.SPLIT if settings.allow_split else fallback


def hard_action(
    settings: PlaySettings,
    total: int,
    upcard: DealerUpcard,
) -> Action:
    """Return a first-pass basic strategy action for a hard total."""
    dealer = dealer_value(upcard)
    dealer_hits_soft_17 = (
        settings.dealer_soft_17_rule == DealerSoft17Rule.HIT
    )

    if total >= 17:
        return Action.STAND

    if total == 16:
        if dealer >= 9:
            return surrender_or_hit(settings)
        return Action.STAND if 2 <= dealer <= 6 else Action.HIT

    if total == 15:
        if dealer == 10 or (dealer == 11 and dealer_hits_soft_17):
            return surrender_or_hit(settings)
        return Action.STAND if 2 <= dealer <= 6 else Action.HIT

    if total >= 13:
        return Action.STAND if 2 <= dealer <= 6 else Action.HIT

    if total == 12:
        return Action.STAND if 4 <= dealer <= 6 else Action.HIT

    if total == 11:
        if dealer == 11 and not dealer_hits_soft_17:
            return Action.HIT
        return double_or_hit(settings, total)

    if total == 10:
        return double_or_hit(settings, total) if 2 <= dealer <= 9 else Action.HIT

    if total == 9:
        return double_or_hit(settings, total) if 3 <= dealer <= 6 else Action.HIT

    return Action.HIT


def soft_action(
    settings: PlaySettings,
    total: int,
    upcard: DealerUpcard,
) -> Action:
    """Return a first-pass basic strategy action for a soft total."""
    dealer = dealer_value(upcard)
    dealer_hits_soft_17 = (
        settings.dealer_soft_17_rule == DealerSoft17Rule.HIT
    )

    if total >= 20:
        return Action.STAND

    if total == 19:
        if dealer == 6 and dealer_hits_soft_17:
            return double_or_hit(settings, total)
        return Action.STAND

    if total == 18:
        if 3 <= dealer <= 6:
            return double_or_hit(settings, total)
        if dealer == 2 and dealer_hits_soft_17:
            return double_or_hit(settings, total)
        return Action.STAND if dealer in (7, 8) else Action.HIT

    if total == 17:
        if 3 <= dealer <= 6:
            return double_or_hit(settings, total)
        if dealer == 2 and dealer_hits_soft_17:
            return double_or_hit(settings, total)
        return Action.HIT

    if total in (15, 16):
        if 4 <= dealer <= 6:
            return double_or_hit(settings, total)
        return Action.HIT

    if total in (13, 14):
        if 5 <= dealer <= 6:
            return double_or_hit(settings, total)
        if dealer == 4 and dealer_hits_soft_17:
            return double_or_hit(settings, total)

    return Action.HIT


def pair_action(
    settings: PlaySettings,
    total: int,
    upcard: DealerUpcard,
) -> Action:
    """Return a first-pass basic strategy action for a pair total."""
    dealer = dealer_value(upcard)
    double_after_split = settings.allow_double_after_split

    if total in (12, 22):
        return split_or_fallback(settings, hard_action(settings, total, upcard))

    if total == 20:
        return Action.STAND

    if total == 18:
        if (2 <= dealer <= 6) or dealer in (8, 9):
            return split_or_fallback(settings, Action.STAND)
        return Action.STAND

    if total == 14:
        if 2 <= dealer <= 7:
            return split_or_fallback(settings, Action.HIT)
        return Action.HIT

    if total == 10:
        return hard_action(settings, total, upcard)

    if total == 8:
        if double_after_split and 5 <= dealer <= 6:
            return split_or_fallback(settings, Action.HIT)
        return Action.HIT

    if total == 6 or total == 4:
        if (4 <= dealer <= 7) or (double_after_split and 2 <= dealer <= 3):
            return split_or_fallback(settings, Action.HIT)
        return Action.HIT

    if total == 16:
        return split_or_fallback(settings, hard_action(settings, total, upcard))

    return hard_action(settings, total, upcard)


def best_action(settings: PlaySettings, context: StrategyContext) -> Action:
    """Return the best first-pass action for a strategy context."""
    if context.hand_kind == HandKind.HARD:
        return hard_action(settings, context.player_total, context.dealer_upcard)

    if context.hand_kind == HandKind.SOFT:
        return soft_action(settings, context.player_total, context.dealer_upcard)

    return pair_action(settings, context.player_total, context.dealer_upcard)


def contexts() -> list[StrategyContext]:
    """Return the first-pass strategy contexts to generate."""
    result: list[StrategyContext] = []

    for total in range(5, 22):
        for upcard in DealerUpcard:
            result.append(StrategyContext(HandKind.HARD, total, upcard))

    for total in range(13, 22):
        for upcard in DealerUpcard:
            result.append(StrategyContext(HandKind.SOFT, total, upcard))

    for total in range(4, 23, 2):
        for upcard in DealerUpcard:
            result.append(StrategyContext(HandKind.PAIR, total, upcard))

    return result


def expected_values(settings: PlaySettings) -> list[ExpectedValue]:
    """Generate first-pass expected value rows."""
    rows: list[ExpectedValue] = []

    for context in contexts():
        selected_action = best_action(settings, context)

        for action in Action:
            rows.append(
                ExpectedValue(
                    hand_kind=context.hand_kind.serialize(),
                    player_total=context.player_total,
                    dealer_upcard=context.dealer_upcard.serialize(),
                    action=action.serialize(),
                    ev=1.0 if action == selected_action else 0.0,
                )
            )

    return rows
