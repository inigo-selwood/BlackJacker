"""Golden expected-value tests for strategy generation."""

from dataclasses import dataclass
from functools import cache
from pathlib import Path
from tempfile import NamedTemporaryFile
from typing import Any

import pytest
import settings
import yaml
from simulation import strategy

_ROOT = Path(__file__).resolve().parents[2]
_RESOURCES = _ROOT / "test" / "resources"
_SMOKE_CASES = _RESOURCES / "smoke_cases"


@dataclass(frozen=True)
class ExpectedValueKey:
    """Lookup key for one generated EV row."""

    hand_kind: str
    player_total: int
    dealer_upcard: str
    action: str


@dataclass(frozen=True)
class ExpectedValueCase:
    """Expected values for one smoke case."""

    available: bool
    ev: float


@dataclass(frozen=True)
class SmokeCase:
    """Smoke test case with its ruleset suite."""

    suite: str
    values: dict[str, Any]


def _load_smoke_cases() -> list[SmokeCase]:
    """Return smoke cases from resource fixtures."""
    smoke_cases: list[SmokeCase] = []

    for path in sorted(_SMOKE_CASES.glob("*.yaml")):
        with path.open("r", encoding="utf-8") as cases_file:
            suite = yaml.safe_load(cases_file)

        for case in suite["cases"]:
            smoke_cases.append(SmokeCase(path.stem, case))

    return smoke_cases


def _case_id(case: SmokeCase) -> str:
    """Return a readable pytest id for a smoke case."""
    return (
        f"{case.suite}-{case.values['hand-kind']}-"
        f"{case.values['player-total']}-v-"
        f"{case.values['dealer-upcard']}-{case.values['action']}"
    )


@cache
def _expected_values(
    suite: str,
) -> dict[ExpectedValueKey, ExpectedValueCase]:
    """Return generated EV rows keyed by strategy context."""
    with (_SMOKE_CASES / f"{suite}.yaml").open(
        "r", encoding="utf-8"
    ) as cases_file:
        suite_data = yaml.safe_load(cases_file)

    with NamedTemporaryFile("w", encoding="utf-8") as settings_file:
        settings_data = {
            key: value for key, value in suite_data.items() if key != "cases"
        }
        yaml.safe_dump(settings_data, settings_file)
        settings_file.flush()
        rules = settings.load(Path(settings_file.name))

    rows = strategy.expected_values(rules)
    return {
        ExpectedValueKey(
            row.hand_kind,
            row.player_total,
            row.dealer_upcard,
            row.action,
        ): ExpectedValueCase(row.available, row.ev)
        for row in rows
    }


@pytest.mark.parametrize(
    "case",
    _load_smoke_cases(),
    ids=_case_id,
)
def test_expected_values_match_resources(
    case: SmokeCase,
) -> None:
    """Compare generated EV rows with hard-coded resource cases."""
    expected_values = _expected_values(case.suite)
    key = ExpectedValueKey(
        case.values["hand-kind"],
        case.values["player-total"],
        case.values["dealer-upcard"],
        case.values["action"],
    )

    assert key in expected_values
    assert expected_values[key].available == case.values["available"]
    assert expected_values[key].ev == pytest.approx(
        case.values["ev"], abs=0.000001
    )
