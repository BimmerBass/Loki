#!/usr/bin/env python3

# Loki, a UCI-compliant chess playing software
# Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
#
# Loki is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

"""Shared support for Fastchess-backed benchmark applications."""

from __future__ import annotations

import asyncio
import hashlib
import json
import os
import re
import shutil
import signal
import subprocess
from pathlib import Path
from typing import Any, Iterable


ENV_RE = re.compile(r"\$\{([A-Za-z_][A-Za-z0-9_]*)\}")
HEADER_RE = re.compile(r'^\[(\w+)\s+"(.*)"\]$')
START_RE = re.compile(r"Started game\s+(\d+)(?:\s+of\s+(\d+))?\s+\((.+?)\s+vs\s+(.+?)\)", re.I)
FINISH_RE = re.compile(
    r"Finished game\s+(\d+)\s+\((.+?)\s+vs\s+(.+?)\):\s+"
    r"(1-0|0-1|1/2-1/2|\*)(?:\s+\{(.*)\})?",
    re.I,
)
ENGINE_FAILURE_WORDS = (
    "internal error", "info string error:", "crash", "illegal move", "disconnect",
    "timed out", "timeout", "time forfeit", "time loss", "protocol", "stalled",
    "loses on time", "forfeit", "failed to", "failure", "unexpected exit", "exited with",
    "doesn't respond",
)

COMMON_MATCH_CSS = """
Screen { layout: vertical; }
#run_view, #final_view { height: 1fr; layout: vertical; }
#summary { height: 4; padding: 0 1; content-align: left middle; }
#progress { height: 3; margin: 0 1; }
#tables { height: 1fr; }
#opponents_panel { width: 48%; min-width: 46; height: 1fr; }
#games_panel { width: 52%; height: 1fr; }
#opponents, #sprt, #games { height: 1fr; margin: 0 1 1 1; }
#event_log { height: 9; margin: 0 1 1 1; border: solid $accent; }
#final_summary { height: 6; padding: 0 1; }
#diagnostics { height: 1fr; margin: 0 1 1 1; border: solid $accent; }
.hidden { display: none; }
"""


class ConfigError(Exception):
    """A user-facing workload, dependency, or setup error."""


def load_env_file(path: Path | None) -> dict[str, str]:
    if path is None:
        return {}
    try:
        values = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ConfigError(f"could not read environment file: {exc}") from exc
    if not isinstance(values, dict) or not all(isinstance(key, str) and isinstance(value, str) for key, value in values.items()):
        raise ConfigError("environment file must be a JSON object containing string values")
    base = path.resolve().parent
    return {key: path_value(base, value) for key, value in values.items()}


def expand_env(value: Any, local_env: dict[str, str]) -> Any:
    if isinstance(value, str):
        def replace(match: re.Match[str]) -> str:
            name = match.group(1)
            if name in local_env:
                return local_env[name]
            if name not in os.environ:
                raise ConfigError(f"environment variable is not set: {name}")
            return os.environ[name]
        return ENV_RE.sub(replace, value)
    if isinstance(value, list):
        return [expand_env(item, local_env) for item in value]
    if isinstance(value, dict):
        return {key: expand_env(item, local_env) for key, item in value.items()}
    return value


def path_value(base: Path, value: str) -> str:
    path = Path(value)
    return str((base / path).resolve() if not path.is_absolute() else path.resolve())


def executable_exists(command: str) -> bool:
    return Path(command).is_file() if any(sep in command for sep in ("/", "\\")) else shutil.which(command) is not None


def safe_name(value: str, fallback: str = "match") -> str:
    return re.sub(r"[^A-Za-z0-9_.-]+", "-", value).strip("-.") or fallback


def engine_args(engine: dict[str, Any]) -> list[str]:
    values = [f"cmd={engine['command']}", f"name={engine['name']}", f"dir={engine['working_directory']}"]
    if engine.get("args"):
        values.append(f"args={subprocess.list2cmdline(engine['args'])}")
    values.extend(f"option.{name}={value}" for name, value in engine.get("uci_options", {}).items())
    return values


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def pgn_blocks(path: Path) -> list[bytes]:
    if not path.is_file():
        return []
    data = path.read_bytes()
    starts = [match.start() for match in re.finditer(br'(?m)^\[Event\s+"', data)]
    blocks: list[bytes] = []
    for index, start in enumerate(starts):
        end = starts[index + 1] if index + 1 < len(starts) else len(data)
        block = data[start:end]
        header = re.search(br'(?m)^\[Result\s+"(1-0|0-1|1/2-1/2|\*)"\]\r?$', block)
        if header and re.search(br"(?:^|\s)(1-0|0-1|1/2-1/2|\*)\s*$", block.rstrip()):
            blocks.append(block.rstrip() + b"\n\n")
    return blocks


def pgn_game_records(path: Path) -> list[dict[str, Any]]:
    """Return complete PGNs together with the headers needed to identify a game."""
    records: list[dict[str, Any]] = []
    for position, block in enumerate(pgn_blocks(path), 1):
        text = block.decode("utf-8", errors="replace").rstrip()
        headers: dict[str, str] = {}
        for raw in text.splitlines():
            match = HEADER_RE.match(raw)
            if match:
                headers[match.group(1)] = match.group(2)
        round_match = re.match(r"\s*(\d+)", headers.get("Round", ""))
        records.append({
            "position": position,
            "round_number": int(round_match.group(1)) if round_match else None,
            "headers": headers,
            "pgn": text,
        })
    return records


def parse_games(path: Path) -> list[dict[str, str]]:
    games: list[dict[str, str]] = []
    for record in pgn_game_records(path):
        headers = record["headers"]
        games.append({
            "white": headers.get("White", "?"), "black": headers.get("Black", "?"),
            "result": headers.get("Result", "*"), "termination": headers.get("Termination", ""),
        })
    return games


def result_stats(games: list[dict[str, str]], subject: str, opponent: str | None = None) -> dict[str, int]:
    selected = [game for game in games if opponent is None or opponent in (game["white"], game["black"])]
    wins = draws = losses = 0
    for game in selected:
        if game["result"] == "1/2-1/2":
            draws += 1
        elif (game["white"] == subject and game["result"] == "1-0") or (game["black"] == subject and game["result"] == "0-1"):
            wins += 1
        else:
            losses += 1
    return {"played": len(selected), "wins": wins, "draws": draws, "losses": losses}


def load_textual() -> dict[str, Any]:
    try:
        from textual.app import App, ComposeResult
        from textual.containers import Container, Horizontal, Vertical, VerticalScroll
        from textual.widgets import DataTable, Footer, Header, ProgressBar, RichLog, Static
    except ImportError as exc:
        raise ConfigError("Textual is not installed. Run: python -m pip install -r benchmark/requirements.txt") from exc
    return locals()


def compose_match_view(textual: dict[str, Any], total: int, left_title: str, left_table_id: str) -> Iterable[Any]:
    Container, Horizontal, Vertical = textual["Container"], textual["Horizontal"], textual["Vertical"]
    VerticalScroll = textual["VerticalScroll"]
    DataTable, Footer, Header = textual["DataTable"], textual["Footer"], textual["Header"]
    ProgressBar, RichLog, Static = textual["ProgressBar"], textual["RichLog"], textual["Static"]
    yield Header(show_clock=True)
    with Container(id="run_view"):
        yield Static(id="summary")
        yield ProgressBar(total=total, id="progress")
        with Horizontal(id="tables"):
            with Vertical(id="opponents_panel"):
                yield Static(left_title)
                yield DataTable(id=left_table_id)
            with Vertical(id="games_panel"):
                yield Static("Recent games")
                yield DataTable(id="games")
        yield RichLog(id="event_log", wrap=True, highlight=True, markup=True)
    with Container(id="final_view", classes="hidden"):
        yield Static(id="final_summary")
        with VerticalScroll():
            yield RichLog(id="diagnostics", wrap=True, highlight=True, markup=True)
    yield Footer()


def configure_game_table(table: Any) -> None:
    for name in ("#", "White", "Black", "Result", "Termination"):
        table.add_column(name, key=name.lower())


def refresh_game_table(table: Any, games: list[dict[str, str]]) -> None:
    table.clear()
    for index, game in list(enumerate(games, 1))[-100:]:
        table.add_row(str(index), game["white"], game["black"], game["result"], game["termination"], key=f"game-{index}")


async def stop_process_tree(proc: asyncio.subprocess.Process | None) -> None:
    if proc is None or proc.returncode is not None:
        return
    if os.name == "nt":
        await asyncio.to_thread(
            subprocess.run, ["taskkill", "/PID", str(proc.pid), "/T", "/F"], capture_output=True, check=False
        )
    else:
        try:
            os.killpg(proc.pid, signal.SIGTERM)
        except ProcessLookupError:
            return
    try:
        await asyncio.wait_for(proc.wait(), timeout=2.0)
    except asyncio.TimeoutError:
        if os.name != "nt":
            try:
                os.killpg(proc.pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
        await proc.wait()
