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
class StrategyRules:
    """BlackJacker play settings relevant to strategy generation."""

    deck_count: int
    allow_double_down: bool
    allow_double_after_split: bool
    double_rule: DoubleRule
    allow_split: bool
    allow_resplit: bool
    allow_hit_split_aces: bool
    allow_resplit_aces: bool
    allow_surrender: bool
    use_no_hole_card_rule: bool
    dealer_soft_17_rule: DealerSoft17Rule
    blackjack_payout: BlackjackPayout


def _required(data: dict[str, Any], key: str) -> Any:
    """Return a required YAML value."""
    if key not in data:
        raise ValueError(f"Missing required setting: {key}")
    return data[key]


def _parse_bool(data: dict[str, Any], key: str) -> bool:
    """Parse a required boolean setting."""
    value = _required(data, key)
    if not isinstance(value, bool):
        raise ValueError(f"Expected boolean for setting: {key}")
    return value


def _parse_int(data: dict[str, Any], key: str) -> int:
    """Parse a required integer setting."""
    value = _required(data, key)
    if not isinstance(value, int) or isinstance(value, bool):
        raise ValueError(f"Expected integer for setting: {key}")
    return value


def _parse_enum(
    data: dict[str, Any],
    key: str,
    enum_type: type[StrEnum],
) -> Any:
    """Parse a required string enum setting."""
    value = _required(data, key)
    if not isinstance(value, str):
        raise ValueError(f"Expected string for setting: {key}")

    try:
        return enum_type(value)
    except ValueError as error:
        raise ValueError(f"Unsupported value for setting: {key}") from error


def load(path: Path) -> StrategyRules:
    """Load strategy-relevant settings from a YAML file."""
    with path.open("r", encoding="utf-8") as settings_file:
        data = yaml.safe_load(settings_file)

    if not isinstance(data, dict):
        raise ValueError("Settings YAML must contain a mapping.")

    rules = data.get("rules", data)

    if not isinstance(rules, dict):
        raise ValueError("Settings YAML rules must contain a mapping.")

    return StrategyRules(
        deck_count=_parse_int(rules, "deck-count"),
        allow_double_down=_parse_bool(rules, "allow-double-down"),
        allow_double_after_split=_parse_bool(
            rules,
            "allow-double-after-split",
        ),
        double_rule=_parse_enum(rules, "double-rule", DoubleRule),
        allow_split=_parse_bool(rules, "allow-split"),
        allow_resplit=_parse_bool(rules, "allow-resplit"),
        allow_hit_split_aces=_parse_bool(rules, "allow-hit-split-aces"),
        allow_resplit_aces=_parse_bool(rules, "allow-resplit-aces"),
        allow_surrender=_parse_bool(rules, "allow-surrender"),
        use_no_hole_card_rule=_parse_bool(rules, "use-no-hole-card-rule"),
        dealer_soft_17_rule=_parse_enum(
            rules,
            "dealer-soft-17-rule",
            DealerSoft17Rule,
        ),
        blackjack_payout=_parse_enum(
            rules,
            "blackjack-payout",
            BlackjackPayout,
        ),
    )
