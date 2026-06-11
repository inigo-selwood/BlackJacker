"""Expected value calculation helpers."""

from settings import PlaySettings
from simulation import dealer
from simulation.table.card import Card


def stand(
    player_total: int,
    dealer_upcard: Card,
    settings: PlaySettings,
) -> float:
    """Return EV for standing on a player total."""
    if player_total > 21:
        return -1.0

    expected_value = 0.0
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
