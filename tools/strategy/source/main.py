"""Generate BlackJacker strategy SQLite artifacts."""

import arguments
import schema
import settings
from simulation import strategy


def main() -> int:
    """Run the strategy generator CLI."""
    args = arguments.parse()
    arguments.validate(args)

    if args.output.exists():
        args.output.unlink()

    try:
        play_settings = settings.load(args.settings)
    except ValueError as error:
        raise SystemExit(str(error)) from error

    schema.create_database(args.output)
    schema.insert_expected_values(
        args.output,
        strategy.expected_values(play_settings),
    )
    print(
        "Created strategy database at "
        f"{args.output}; generated exact EVs without resplit recursion."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
