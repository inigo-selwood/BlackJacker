"""SQLAlchemy ORM schema for strategy database artifacts."""

from pathlib import Path

from sqlalchemy import Boolean, Enum, Float, Integer, create_engine
from sqlalchemy.orm import DeclarativeBase, Mapped, Session, mapped_column

HandKindType = Enum("hard", "soft", "pair", name="hand_kind")
DealerUpcardType = Enum(
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "ace",
    name="dealer_upcard",
)
ActionType = Enum(
    "hit",
    "stand",
    "double",
    "double-hit",
    "double-stand",
    "split",
    "split-hit",
    "split-stand",
    "surrender",
    "surrender-hit",
    "surrender-stand",
    name="action",
)


class _Base(DeclarativeBase):
    """Base class for strategy database ORM models."""


class ExpectedValue(_Base):
    """Expected value for one action in one strategy decision context."""

    __tablename__ = "expected_values"

    id: Mapped[int] = mapped_column(Integer, primary_key=True)
    hand_kind: Mapped[str] = mapped_column(HandKindType, nullable=False)
    player_total: Mapped[int] = mapped_column(Integer, nullable=False)
    dealer_upcard: Mapped[str] = mapped_column(
        DealerUpcardType,
        nullable=False,
    )
    action: Mapped[str] = mapped_column(ActionType, nullable=False)
    available: Mapped[bool] = mapped_column(Boolean, nullable=False)
    ev: Mapped[float] = mapped_column(Float, nullable=False)


def create_database(output_path: Path) -> None:
    """Create an empty strategy database schema."""
    engine = create_engine(f"sqlite:///{output_path}")
    _Base.metadata.create_all(engine)


def insert_expected_values(
    output_path: Path,
    expected_values: list[ExpectedValue],
) -> None:
    """Insert expected value rows into a strategy database."""
    engine = create_engine(f"sqlite:///{output_path}")

    with Session(engine) as session:
        session.add_all(expected_values)
        session.commit()
