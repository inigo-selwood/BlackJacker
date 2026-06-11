"""ORM value conversion helpers."""

from enum import StrEnum
from typing import TypeVar

from simulation.table.card import Rank

OrmEnum = TypeVar("OrmEnum", bound=StrEnum)


def to_orm(value: Rank | StrEnum) -> str:
    """Return the SQLAlchemy value for a supported domain value."""
    if isinstance(value, Rank):
        if value == Rank.ACE:
            return "ace"

        if value >= Rank.TEN:
            return "10"

        return str(int(value))

    return value.value


def from_orm(
    value_type: type[Rank] | type[OrmEnum], value: str
) -> Rank | OrmEnum:
    """Return a supported domain value from a SQLAlchemy value."""
    if value_type == Rank:
        if value == "ace":
            return Rank.ACE

        if value == "10":
            return Rank.TEN

        return Rank(int(value))

    return value_type(value)
