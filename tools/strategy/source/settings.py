"""Settings YAML ingestion for the strategy generator."""

from dataclasses import dataclass
from enum import StrEnum
from pathlib import Path
from typing import Any

import yaml


class DoubleRule(StrEnum):
    """Allowed double-down totals."""

    ANY_TWO = "any-two"
    NINE_TEN_ELEVEN = "9-10-11"
    TEN_ELEVEN = "10-11"


class DealerSoft17Rule(StrEnum):
    """Dealer soft-17 behavior."""

    HIT = "hit"
    STAND = "stand"


class BlackjackPayout(StrEnum):
    """Natural blackjack payout."""

    THREE_TO_TWO = "3-to-2"
    SIX_TO_FIVE = "6-to-5"


@dataclass(frozen=True)
class PlaySettings:
    """BlackJacker play settings relevant to strategy generation."""

    deck_count: int
    cut_percent: int
    allow_double_down: bool
    allow_double_after_split: bool
    double_rule: DoubleRule
    allow_split: bool
    allow_resplit: bool
    allow_hit_split_aces: bool
    allow_resplit_aces: bool
    allow_surrender: bool
    use_no_hole_card_rule: bool
    show_true_count: bool
    dealer_soft_17_rule: DealerSoft17Rule
    blackjack_payout: BlackjackPayout


def required(data: dict[str, Any], key: str) -> Any:
    """Return a required YAML value."""
    if key not in data:
        raise ValueError(f"Missing required setting: {key}")
    return data[key]


def parse_bool(data: dict[str, Any], key: str) -> bool:
    """Parse a required boolean setting."""
    value = required(data, key)
    if not isinstance(value, bool):
        raise ValueError(f"Expected boolean for setting: {key}")
    return value


def parse_int(data: dict[str, Any], key: str) -> int:
    """Parse a required integer setting."""
    value = required(data, key)
    if not isinstance(value, int) or isinstance(value, bool):
        raise ValueError(f"Expected integer for setting: {key}")
    return value


def parse_enum(
    data: dict[str, Any],
    key: str,
    enum_type: type[StrEnum],
) -> Any:
    """Parse a required string enum setting."""
    value = required(data, key)
    if not isinstance(value, str):
        raise ValueError(f"Expected string for setting: {key}")

    try:
        return enum_type(value)
    except ValueError as error:
        raise ValueError(f"Unsupported value for setting: {key}") from error


def load(path: Path) -> PlaySettings:
    """Load strategy-relevant settings from a YAML file."""
    with path.open("r", encoding="utf-8") as settings_file:
        data = yaml.safe_load(settings_file)

    if not isinstance(data, dict):
        raise ValueError("Settings YAML must contain a mapping.")

    return PlaySettings(
        deck_count=parse_int(data, "deck-count"),
        cut_percent=parse_int(data, "cut-percent"),
        allow_double_down=parse_bool(data, "allow-double-down"),
        allow_double_after_split=parse_bool(
            data,
            "allow-double-after-split",
        ),
        double_rule=parse_enum(data, "double-rule", DoubleRule),
        allow_split=parse_bool(data, "allow-split"),
        allow_resplit=parse_bool(data, "allow-resplit"),
        allow_hit_split_aces=parse_bool(data, "allow-hit-split-aces"),
        allow_resplit_aces=parse_bool(data, "allow-resplit-aces"),
        allow_surrender=parse_bool(data, "allow-surrender"),
        use_no_hole_card_rule=parse_bool(data, "use-no-hole-card-rule"),
        show_true_count=parse_bool(data, "show-true-count"),
        dealer_soft_17_rule=parse_enum(
            data,
            "dealer-soft-17-rule",
            DealerSoft17Rule,
        ),
        blackjack_payout=parse_enum(
            data,
            "blackjack-payout",
            BlackjackPayout,
        ),
    )
