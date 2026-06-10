"""Command-line argument parsing for the strategy generator."""

import argparse
from dataclasses import dataclass
from pathlib import Path
from typing import Sequence


@dataclass(frozen=True)
class Arguments:
    """Validated command-line arguments."""

    settings: Path
    output: Path
    force: bool


def parse(argv: Sequence[str] | None = None) -> Arguments:
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(
        description=(
            "Generate a BlackJacker strategy SQLite database from a settings "
            "YAML file."
        )
    )
    parser.add_argument(
        "settings",
        type=Path,
        help="Path to the BlackJacker settings YAML file.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("output.sqlite3"),
        help=(
            "Path to the output SQLite database. Use a .sqlite3 extension. "
            "Defaults to output.sqlite3."
        ),
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Overwrite the output database if it already exists.",
    )
    namespace = parser.parse_args(argv)
    return Arguments(
        settings=namespace.settings,
        output=namespace.output,
        force=namespace.force,
    )


def validate(args: Arguments) -> None:
    """Validate input and output paths."""
    if not args.settings.is_file():
        raise SystemExit(f"Settings file not found: {args.settings}")

    if args.output.suffix != ".sqlite3":
        raise SystemExit("Output path must use a .sqlite3 extension.")

    if args.output.exists() and not args.force:
        raise SystemExit(
            f"Output already exists: {args.output}. Use --force to overwrite."
        )

    args.output.parent.mkdir(parents=True, exist_ok=True)
