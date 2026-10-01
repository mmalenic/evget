"""
Reads the data that evget stored.
"""

from __future__ import annotations

import json
import sqlite3
from contextlib import closing
from pathlib import Path
from typing import Any

Row = dict[str, Any]
Tables = dict[str, list[Row]]

TABLES = {
    "Key": "key",
    "MouseClick": "mouse_click",
    "MouseMove": "mouse_move",
    "MouseScroll": "mouse_scroll",
}


def read(path: Path) -> Tables:
    """
    Read an evget output file.
    """

    if path.suffix == ".sqlite":
        return read_sqlite(path)

    return read_json(path.read_text(encoding="utf-8"))


def read_sqlite(path: Path) -> Tables:
    """
    Read a database.
    """

    with closing(sqlite3.connect(f"{path.as_uri()}?mode=ro", uri=True)) as db:
        db.row_factory = sqlite3.Row
        enums = {
            column: dict(db.execute(f"select id, enum from {column}").fetchall())
            for column in ("button_action", "device_type")
        }
        tables: Tables = {}

        for table in TABLES.values():
            rows = [dict(row) for row in db.execute(f"select * from {table}")]
            for row in rows:
                for column, names in enums.items():
                    if column in row:
                        row[column] = names[row[column]]
            tables[table] = rows

        return tables


def read_json(text: str) -> Tables:
    """
    Read JSON output.
    """

    tables: Tables = {table: [] for table in TABLES.values()}
    decoder = json.JSONDecoder()

    index = _skip_space(text, 0)
    while index < len(text):
        document, index = decoder.raw_decode(text, index)
        for entry in document["entries"]:
            fields = {field["name"]: field["data"] for field in entry["fields"]}
            tables[TABLES[entry["type"]]].append(fields)
        index = _skip_space(text, index)

    return tables


def from_device(rows: list[Row], device_name: str) -> list[Row]:
    """
    The rows for `device_name`.
    """

    return [row for row in rows if row["device_name"] == device_name]


def count(path: Path, table: str, device_name: str) -> int:
    """
    Counts the number of rows in `table` at `path` of `device_name`.
    """

    return len(from_device(read(path)[table], device_name))


def integrity(path: Path) -> str:
    """
    Run the SQLite integrity check.
    """

    # Should be rw, so SQLite can roll back a struck transaction if present.
    with closing(sqlite3.connect(f"{path.as_uri()}?mode=rw", uri=True)) as db:
        return db.execute("pragma integrity_check").fetchone()[0]


def _skip_space(text: str, index: int) -> int:
    while index < len(text) and text[index].isspace():
        index += 1
    return index
