# BlackJacker Strategy Tool

The strategy tool generates a SQLite database of expected values for blackjack
decision contexts. It reads the `rules` section from a YAML file and ignores
the `settings` section, which is reserved for trainer/play-mode concerns.

## YAML Shape

```yaml
rules:
  deck-count: 6
  allow-double-down: true
  allow-double-after-split: true
  double-rule: any-two
  allow-split: true
  allow-resplit: true
  allow-hit-split-aces: false
  allow-resplit-aces: true
  allow-surrender: true
  use-no-hole-card-rule: false
  dealer-soft-17-rule: stand
  blackjack-payout: 3-to-2

settings:
  cut-percent: 75
  show-true-count: true
```

## Current EV Model

Currently modelled:

- dealer soft-17 rule
- double-down availability and double-total restrictions
- double after split
- split, resplit, split aces, and resplit aces
- surrender availability

Parsed but not fully modelled yet:

- `deck-count`: EV currently uses infinite-deck rank probabilities.
- `blackjack-payout`: natural blackjack payout is not part of the current
  post-deal action EV model.
- `use-no-hole-card-rule`: no-hole-card exposure is not modelled yet.

The generated database still includes one row per action/context. Illegal
actions have `available = false` and `ev = 0.0`; consumers should treat
`available` as authoritative.
