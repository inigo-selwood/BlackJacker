"""Golden expected-value tests for strategy generation."""

from dataclasses import dataclass
from pathlib import Path

import pytest
import settings
import yaml
from simulation import strategy

_ROOT = Path(__file__).resolve().parents[2]
_RESOURCES = _ROOT / "test" / "resources"


@dataclass(frozen=True)
class ExpectedValueKey:
    """Lookup key for one generated EV row."""

    hand_kind: str
    player_total: int
    dealer_upcard: str
    action: str


def _load_smoke_cases() -> list[dict[str, object]]:
    """Return smoke cases from the resource fixture."""
    with (_RESOURCES / "smoke_cases.yaml").open(
        "r", encoding="utf-8"
    ) as cases_file:
        return yaml.safe_load(cases_file)


def _case_id(case: dict[str, object]) -> str:
    """Return a readable pytest id for a smoke case."""
    return (
        f"{case['hand-kind']}-{case['player-total']}-"
        f"v-{case['dealer-upcard']}-{case['action']}"
    )


@pytest.fixture(scope="module")
def expected_values() -> dict[ExpectedValueKey, float]:
    """Return generated EV rows keyed by strategy context."""
    play_settings = settings.load(_RESOURCES / "settings.yaml")
    rows = strategy.expected_values(play_settings)
    return {
        ExpectedValueKey(
            row.hand_kind,
            row.player_total,
            row.dealer_upcard,
            row.action,
        ): row.ev
        for row in rows
    }


@pytest.mark.parametrize(
    "case",
    _load_smoke_cases(),
    ids=_case_id,
)
def test_expected_values_match_resources(
    expected_values: dict[ExpectedValueKey, float],
    case: dict[str, object],
) -> None:
    """Compare generated EV rows with hard-coded resource cases."""
    key = ExpectedValueKey(
        case["hand-kind"],
        case["player-total"],
        case["dealer-upcard"],
        case["action"],
    )

    assert key in expected_values
    assert expected_values[key] == pytest.approx(case["ev"], abs=0.000001)
