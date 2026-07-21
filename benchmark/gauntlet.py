#!/usr/bin/env python3

# Loki, a UCI-compliant chess playing software
# Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
#
# Loki is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Loki is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#


from __future__ import annotations

import argparse
import asyncio
import csv
import hashlib
import json
import os
import re
import subprocess
import sys
import time
from datetime import datetime
from pathlib import Path
from typing import Any, Callable

from match_runner import (
    COMMON_MATCH_CSS,
    ENGINE_FAILURE_WORDS as LOKI_FAILURE_WORDS,
    FINISH_RE,
    START_RE,
    ConfigError,
    compose_match_view,
    configure_game_table,
    engine_args,
    executable_exists,
    expand_env,
    load_env_file,
    load_textual,
    parse_games,
    path_value,
    pgn_blocks,
    refresh_game_table,
    result_stats,
    safe_name,
    stop_process_tree,
)


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run a resumable Loki Elo gauntlet with fastchess and Ordo.")
    parser.add_argument("--workload", required=True, type=Path, help="JSON workload definition.")
    parser.add_argument("--env-file", type=Path, help="Local JSON values for ${NAME} placeholders.")
    parser.add_argument("--resume-state", type=Path, help="gauntlet-state.json from a paused run.")
    parser.add_argument("--output-dir", type=Path, help="Output directory for a new run.")
    return parser.parse_args(argv)


def load_workload(path: Path, local_env: dict[str, str] | None = None) -> tuple[dict[str, Any], list[str]]:
    try:
        raw = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ConfigError(f"could not read workload: {exc}") from exc
    if not isinstance(raw, dict):
        raise ConfigError("workload root must be a JSON object")
    cfg = expand_env(raw, local_env or {})
    if cfg.get("schema_version") != 1:
        raise ConfigError("schema_version must be 1")

    for key in ("name", "tools", "subject", "opponents", "match"):
        if key not in cfg:
            raise ConfigError(f"missing workload field: {key}")
    if not isinstance(cfg["opponents"], list) or not cfg["opponents"]:
        raise ConfigError("opponents must contain at least one engine")

    base = path.resolve().parent
    tools = cfg["tools"]
    for name in ("fastchess", "ordo"):
        if not isinstance(tools.get(name), str) or not tools[name]:
            raise ConfigError(f"tools.{name} must be an executable path or command")
        if any(sep in tools[name] for sep in ("/", "\\")):
            tools[name] = path_value(base, tools[name])

    engines = [cfg["subject"], *cfg["opponents"]]
    names: set[str] = set()
    for index, engine in enumerate(engines):
        label = "subject" if index == 0 else f"opponents[{index - 1}]"
        if not isinstance(engine, dict):
            raise ConfigError(f"{label} must be an object")
        name = engine.get("name")
        command = engine.get("command")
        if not isinstance(name, str) or not name.strip():
            raise ConfigError(f"{label}.name is required")
        if name in names:
            raise ConfigError(f"duplicate engine name: {name}")
        names.add(name)
        if not isinstance(command, str) or not command:
            raise ConfigError(f"{label}.command is required")
        engine["command"] = path_value(base, command)
        engine["args"] = engine.get("args", [])
        engine["uci_options"] = engine.get("uci_options", {})
        engine["working_directory"] = path_value(base, engine.get("working_directory", "."))
        if not isinstance(engine["args"], list) or not all(isinstance(arg, str) for arg in engine["args"]):
            raise ConfigError(f"{label}.args must be a list of strings")
        if not isinstance(engine["uci_options"], dict):
            raise ConfigError(f"{label}.uci_options must be an object")

    warnings: list[str] = []
    sources: set[str] = set()
    for index, opponent in enumerate(cfg["opponents"]):
        rating = opponent.get("rating")
        if not isinstance(rating, dict) or not isinstance(rating.get("elo"), (int, float)):
            raise ConfigError(f"opponents[{index}].rating.elo is required")
        if not rating.get("source"):
            warnings.append(f"{opponent['name']}: rating source is missing")
        else:
            sources.add(str(rating["source"]))
        if not rating.get("retrieved"):
            warnings.append(f"{opponent['name']}: rating retrieval date is missing")
    if len(sources) > 1:
        warnings.append("opponents use ratings from different source lists")

    match = cfg["match"]
    games = match.get("games_per_opponent")
    concurrency = match.get("concurrency", 1)
    if not isinstance(games, int) or games < 2 or games % 2:
        raise ConfigError("match.games_per_opponent must be a positive even integer")
    if not isinstance(concurrency, int) or concurrency < 1:
        raise ConfigError("match.concurrency must be a positive integer")
    if not isinstance(match.get("time_control"), str) or not match["time_control"]:
        raise ConfigError("match.time_control is required")
    openings = match.get("openings")
    if not isinstance(openings, dict) or not openings.get("path"):
        raise ConfigError("match.openings.path is required")
    openings["path"] = path_value(base, openings["path"])
    openings.setdefault("format", Path(openings["path"]).suffix.lstrip(".").lower())
    if openings["format"] not in ("epd", "pgn"):
        raise ConfigError("match.openings.format must be epd or pgn")
    openings.setdefault("order", "sequential")
    if openings["order"] not in ("sequential", "random"):
        raise ConfigError("match.openings.order must be sequential or random")
    match.setdefault("concurrency", 1)
    match.setdefault("time_margin_ms", 1000)
    match.setdefault("recover", False)
    cfg.setdefault("rating", {}).setdefault("interval_games", 20)
    if not isinstance(cfg["rating"]["interval_games"], int) or cfg["rating"]["interval_games"] < 1:
        raise ConfigError("rating.interval_games must be a positive integer")
    return cfg, warnings


def validate_paths(cfg: dict[str, Any]) -> None:
    for tool in ("fastchess", "ordo"):
        if not executable_exists(cfg["tools"][tool]):
            raise ConfigError(f"{tool} executable does not exist: {cfg['tools'][tool]}")
    for engine in [cfg["subject"], *cfg["opponents"]]:
        if not Path(engine["command"]).is_file():
            raise ConfigError(f"engine executable does not exist: {engine['command']}")
        if not Path(engine["working_directory"]).is_dir():
            raise ConfigError(f"engine working directory does not exist: {engine['working_directory']}")
    if not Path(cfg["match"]["openings"]["path"]).is_file():
        raise ConfigError(f"opening suite does not exist: {cfg['match']['openings']['path']}")


def workload_fingerprint(cfg: dict[str, Any]) -> str:
    stable = json.loads(json.dumps(cfg))
    stable["tools"] = {"fastchess": "", "ordo": ""}
    for engine in [stable["subject"], *stable["opponents"]]:
        engine["command"] = ""
        engine["working_directory"] = ""
    encoded = json.dumps(stable, sort_keys=True, separators=(",", ":")).encode()
    return hashlib.sha256(encoded).hexdigest()


def build_command(cfg: dict[str, Any], paths: dict[str, Path], resume: bool) -> list[str]:
    if resume:
        return [cfg["tools"]["fastchess"], "-config", f"file={paths['fastchess_state']}", "stats=true"]
    match = cfg["match"]
    openings = match["openings"]
    command = [cfg["tools"]["fastchess"]]
    for engine in [cfg["subject"], *cfg["opponents"]]:
        command.extend(["-engine", *engine_args(engine)])
    opening_args = [f"file={openings['path']}", f"format={openings['format']}", f"order={openings['order']}"]
    if openings.get("plies") is not None:
        opening_args.append(f"plies={openings['plies']}")
    command.extend([
        "-tournament", "gauntlet", "-seeds", "1",
        "-rounds", str(match["games_per_opponent"] // 2), "-games", "2", "-repeat",
        "-concurrency", str(match["concurrency"]),
        "-each", f"tc={match['time_control']}", f"timemargin={match['time_margin_ms']}",
        "-openings", *opening_args,
        "-pgnout", f"file={paths['pgn']}", "notation=san", "append=false",
        "-log", f"file={paths['trace']}", "level=trace", "append=false", "realtime=true", "engine=true",
        "-config", f"outname={paths['fastchess_state']}", "stats=true",
        "-autosaveinterval", "1",
    ])
    if openings.get("seed") is not None:
        command.extend(["-srand", str(openings["seed"])])
    if match.get("max_moves") is not None:
        command.extend(["-maxmoves", str(match["max_moves"])])
    for option in ("draw", "resign"):
        if isinstance(match.get(option), dict):
            command.append(f"-{option}")
            command.extend(f"{key}={value}" for key, value in match[option].items())
    if match["recover"]:
        command.append("-recover")
    return command


def write_anchors(cfg: dict[str, Any], path: Path) -> None:
    lines = [f'"{opponent["name"].replace(chr(34), chr(34) * 2)}",{opponent["rating"]["elo"]}' for opponent in cfg["opponents"]]
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def update_fastchess_paths(state_path: Path, previous: dict[str, Any], current: dict[str, Any]) -> None:
    replacements: dict[str, str] = {}
    old_by_name = {engine["name"]: engine for engine in [previous["subject"], *previous["opponents"]]}
    for engine in [current["subject"], *current["opponents"]]:
        old = old_by_name.get(engine["name"], {})
        for field in ("command", "working_directory"):
            if old.get(field) and old[field] != engine[field]:
                replacements[str(old[field])] = str(engine[field])
    if not replacements:
        return
    try:
        state = json.loads(state_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ConfigError(f"could not update engine paths in fastchess state: {exc}") from exc

    def replace(value: Any) -> Any:
        if isinstance(value, str):
            for old, new in replacements.items():
                value = value.replace(old, new)
            return value
        if isinstance(value, list):
            return [replace(item) for item in value]
        if isinstance(value, dict):
            return {key: replace(item) for key, item in value.items()}
        return value

    state_path.write_text(json.dumps(replace(state), indent=2), encoding="utf-8")


def parse_ordo(csv_path: Path, text_path: Path, subject: str) -> tuple[float, float | None]:
    if csv_path.is_file():
        rows = list(csv.reader(csv_path.read_text(encoding="utf-8", errors="replace").splitlines()))
        if rows:
            header = [cell.strip().lower() for cell in rows[0]]
            name_at = next((i for i, value in enumerate(header) if value in ("name", "player", "engine")), None)
            rating_at = next((i for i, value in enumerate(header) if "rating" in value), None)
            error_at = next((i for i, value in enumerate(header) if "error" in value), None)
            if name_at is not None and rating_at is not None:
                for row in rows[1:]:
                    if len(row) > max(name_at, rating_at) and row[name_at].strip() == subject:
                        error = float(row[error_at]) if error_at is not None and len(row) > error_at and row[error_at].strip(" -") else None
                        return float(row[rating_at]), error
    if text_path.is_file():
        for line in text_path.read_text(encoding="utf-8", errors="replace").splitlines():
            if subject in line and ":" in line:
                numbers = re.findall(r"[-+]?\d+(?:\.\d+)?", line.split(":", 1)[1])
                if numbers:
                    return float(numbers[0]), float(numbers[1]) if len(numbers) > 1 else None
    raise ValueError(f"could not find an Ordo rating for {subject}")


def make_app(
    textual: dict[str, Any], cfg: dict[str, Any], paths: dict[str, Path], command: list[str],
    warnings: list[str], resume_state: dict[str, Any] | None,
):
    App, ComposeResult = textual["App"], textual["ComposeResult"]
    DataTable = textual["DataTable"]
    ProgressBar, RichLog, Static = textual["ProgressBar"], textual["RichLog"], textual["Static"]

    class GauntletApp(App):
        CSS = COMMON_MATCH_CSS
        BINDINGS = [("p", "pause", "Pause/save"), ("q", "quit", "Quit"), ("ctrl+c", "quit", "Cancel/Quit")]

        def __init__(self) -> None:
            super().__init__()
            self.proc: asyncio.subprocess.Process | None = None
            self.run_task: asyncio.Task[None] | None = None
            self.trace_task: asyncio.Task[None] | None = None
            self.ordo_task: asyncio.Task[None] | None = None
            self.stop_reason: str | None = None
            self.finished = False
            self.finalizing = False
            self.started = time.monotonic()
            self.games: list[dict[str, str]] = []
            self.warning_count = len(warnings)
            self.loki_errors: list[str] = []
            self.diagnostics = list(warnings)
            self.seen_errors: set[str] = set()
            self.capture_loki_lines = 0
            self.rating: float | None = None
            self.rating_error: float | None = None
            self.compromised = False
            self.last_ordo_games = 0
            self.exit_code = 2
            self.total = cfg["match"]["games_per_opponent"] * len(cfg["opponents"])

        def compose(self) -> ComposeResult:
            yield from compose_match_view(textual, self.total, "Opponents", "opponents")

        async def on_mount(self) -> None:
            self.title = "Loki Elo Gauntlet"
            opponent_table = self.query_one("#opponents", DataTable)
            for name in ("Engine", "Anchor", "Played", "W", "D", "L", "Score"):
                opponent_table.add_column(name, key=name.lower())
            for opponent in cfg["opponents"]:
                opponent_table.add_row(opponent["name"], str(opponent["rating"]["elo"]), "0", "0", "0", "0", "-", key=opponent["name"])
            game_table = self.query_one("#games", DataTable)
            configure_game_table(game_table)
            self._refresh_games()
            for warning in warnings:
                self._log(f"[yellow]Warning: {warning}[/yellow]")
            self._log(f"{'Resuming' if resume_state else 'Starting'} {self.total}-game gauntlet")
            self._render_summary()
            self.run_task = asyncio.create_task(self._run())

        def _log(self, message: str) -> None:
            self.query_one("#event_log", RichLog).write(f"[{time.strftime('%H:%M:%S')}] {message}")

        def _render_summary(self) -> None:
            subject = cfg["subject"]["name"]
            stats = result_stats(self.games, subject)
            elo = "pending" if self.rating is None else f"{self.rating:.1f}" + (f" ± {self.rating_error:.1f}" if self.rating_error is not None else "")
            suffix = " (compromised)" if self.compromised else ""
            self.query_one("#summary", Static).update(
                f"Games {len(self.games)}/{self.total} | W/D/L {stats['wins']}/{stats['draws']}/{stats['losses']} | "
                f"Warnings {self.warning_count} | Loki errors {len(self.loki_errors)}\n"
                f"Local CCRL-calibrated Elo {elo}{suffix} | Wall {time.monotonic() - self.started:.1f}s | "
                "p pause/save | q cancel"
            )

        def _refresh_games(self) -> None:
            self.games = parse_games(paths["pgn"])
            self.query_one("#progress", ProgressBar).update(progress=min(len(self.games), self.total))
            subject = cfg["subject"]["name"]
            table = self.query_one("#opponents", DataTable)
            for opponent in cfg["opponents"]:
                name = opponent["name"]
                stats = result_stats(self.games, subject, name)
                values = {
                    "played": stats["played"], "w": stats["wins"], "d": stats["draws"], "l": stats["losses"],
                    "score": f"{(stats['wins'] + stats['draws'] / 2) / stats['played']:.1%}" if stats["played"] else "-",
                }
                for key, value in values.items():
                    table.update_cell(name, key, str(value))
            recent = self.query_one("#games", DataTable)
            refresh_game_table(recent, self.games)
            self._render_summary()

        def _game_log(self, line: str) -> None:
            with paths["game_log"].open("a", encoding="utf-8") as handle:
                handle.write(f"[{datetime.now().astimezone().isoformat(timespec='seconds')}] {line}\n")

        def _record_diagnostic(self, message: str, error: bool = False) -> None:
            self.diagnostics.append(message)
            if not error:
                self.warning_count += 1
            self._log(f"[{'red' if error else 'yellow'}]{'Error' if error else 'Warning'}: {message}[/]")
            self._render_summary()

        async def _read_stream(self, stream: asyncio.StreamReader, source: str) -> None:
            while raw := await stream.readline():
                line = raw.decode(errors="replace").rstrip()
                if START_RE.search(line) or FINISH_RE.search(line):
                    self._game_log(line)
                    self._log(line)
                if FINISH_RE.search(line):
                    await asyncio.sleep(0.05)
                    self._refresh_games()
                    if len(self.games) - self.last_ordo_games >= cfg["rating"]["interval_games"]:
                        self._schedule_ordo()
                lower = line.lower()
                if source == "stderr" or "warning" in lower or "error" in lower or "fatal" in lower:
                    self._classify_line(line)

        def _classify_line(self, line: str) -> None:
            normalized = line.strip()
            if not normalized or normalized in self.seen_errors:
                return
            lower = normalized.lower()
            subject = cfg["subject"]["name"].lower()
            if self.capture_loki_lines and subject in lower:
                with paths["loki_errors"].open("a", encoding="utf-8") as handle:
                    handle.write(f"[{datetime.now().astimezone().isoformat(timespec='seconds')}] {normalized}\n")
                    handle.flush()
                self.capture_loki_lines -= 1
                return
            finish = FINISH_RE.search(normalized)
            if finish and any(word in lower for word in LOKI_FAILURE_WORDS):
                white, black, result = finish.group(2).lower(), finish.group(3).lower(), finish.group(4)
                loser = white if result == "0-1" else black if result == "1-0" else ""
                is_loki = loser == subject
            else:
                is_loki = subject in lower and any(word in lower for word in LOKI_FAILURE_WORDS)
            if "info string error:" in lower or "internal error" in lower:
                is_loki = subject in lower or f"<{subject}(" in lower
            if is_loki:
                self.seen_errors.add(normalized)
                self.loki_errors.append(normalized)
                self.compromised = True
                with paths["loki_errors"].open("a", encoding="utf-8") as handle:
                    handle.write(f"[{datetime.now().astimezone().isoformat(timespec='seconds')}] {normalized}\n")
                    handle.flush()
                if "internal error" in lower:
                    self.capture_loki_lines = 100
                self._record_diagnostic(normalized, error=True)
                if not cfg["match"]["recover"] and self.stop_reason is None:
                    self.stop_reason = "failed"
                    asyncio.create_task(self._stop_process(delay=0.5))
            elif any(word in lower for word in (
                "warning", "error", "fatal", "failed", "illegal", "timeout", "crash",
                "loses on time", "forfeit", "disconnect", "stalled", "protocol",
            )):
                self.seen_errors.add(normalized)
                self._record_diagnostic(normalized, error="fatal" in lower or "error" in lower)

        async def _tail_trace(self) -> None:
            offset = 0
            while self.proc is not None:
                if paths["trace"].is_file():
                    with paths["trace"].open("r", encoding="utf-8", errors="replace") as handle:
                        handle.seek(offset)
                        for line in handle:
                            self._classify_line(line.rstrip())
                        offset = handle.tell()
                if self.proc.returncode is not None:
                    break
                await asyncio.sleep(0.1)

        def _schedule_ordo(self) -> None:
            if self.ordo_task is None or self.ordo_task.done():
                self.ordo_task = asyncio.create_task(self._run_ordo())

        async def _run_ordo(self) -> None:
            blocks = pgn_blocks(paths["pgn"])
            if not blocks:
                return
            paths["ordo_input"].write_bytes(b"".join(blocks))
            cmd = [cfg["tools"]["ordo"], "-Q", "-D", "-m", str(paths["anchors"]), "-p", str(paths["ordo_input"]), "-o", str(paths["ordo_text"]), "-c", str(paths["ordo_csv"])]
            try:
                proc = await asyncio.create_subprocess_exec(*cmd, stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE)
                stdout, stderr = await proc.communicate()
                if proc.returncode:
                    message = (stderr or stdout).decode(errors="replace").strip() or f"Ordo exited with code {proc.returncode}"
                    self._record_diagnostic(message, error=True)
                    return
                self.rating, self.rating_error = parse_ordo(paths["ordo_csv"], paths["ordo_text"], cfg["subject"]["name"])
                self.last_ordo_games = len(blocks)
                self._log(f"Ordo: {self.rating:.1f}" + (f" ± {self.rating_error:.1f}" if self.rating_error is not None else ""))
                self._render_summary()
            except (OSError, ValueError) as exc:
                self._record_diagnostic(f"Ordo failed: {exc}", error=True)

        async def _run(self) -> None:
            flags = subprocess.CREATE_NEW_PROCESS_GROUP if os.name == "nt" else 0
            kwargs: dict[str, Any] = {"creationflags": flags} if os.name == "nt" else {"start_new_session": True}
            try:
                self.proc = await asyncio.create_subprocess_exec(
                    *command, stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE, **kwargs
                )
                assert self.proc.stdout is not None and self.proc.stderr is not None
                self.trace_task = asyncio.create_task(self._tail_trace())
                readers = [asyncio.create_task(self._read_stream(self.proc.stdout, "stdout")), asyncio.create_task(self._read_stream(self.proc.stderr, "stderr"))]
                code = await self.proc.wait()
                await asyncio.gather(*readers, return_exceptions=True)
                if self.trace_task:
                    await self.trace_task
                self._refresh_games()
                if self.stop_reason is None and code:
                    self.stop_reason = "failed"
                    self._record_diagnostic(f"fastchess exited with code {code}", error=True)
            except OSError as exc:
                self.stop_reason = "failed"
                self._record_diagnostic(f"could not start fastchess: {exc}", error=True)
            await self._finish()

        async def _stop_process(self, delay: float = 0.0) -> None:
            if delay:
                await asyncio.sleep(delay)
            await stop_process_tree(self.proc)

        async def action_pause(self) -> None:
            if self.finished or self.finalizing:
                return
            self.stop_reason = "paused"
            self._log("Pause requested; stopping active games and saving state...")
            await self._stop_process()

        async def action_quit(self) -> None:
            if self.finished:
                self.exit(self.exit_code)
                return
            if self.finalizing:
                return
            self.stop_reason = "canceled"
            self._log("Cancellation requested...")
            await self._stop_process()

        async def _finish(self) -> None:
            if self.finished or self.finalizing:
                return
            self.finalizing = True
            blocks = pgn_blocks(paths["pgn"])
            paths["pgn"].write_bytes(b"".join(blocks))
            self._refresh_games()
            if self.ordo_task and not self.ordo_task.done():
                await self.ordo_task
            await self._run_ordo()
            status = self.stop_reason or ("complete" if len(self.games) >= self.total else "failed")
            if status == "paused" and not paths["fastchess_state"].is_file():
                self._record_diagnostic("fastchess did not create resumable state", error=True)
                status = "failed"
            if status == "paused":
                state = {
                    "schema_version": 1, "status": "paused", "run_dir": str(paths["run_dir"]),
                    "workload_fingerprint": workload_fingerprint(cfg), "fastchess_state": str(paths["fastchess_state"]),
                    "completed_games": len(self.games), "pgn_bytes": paths["pgn"].stat().st_size if paths["pgn"].exists() else 0,
                }
                paths["manager_state"].write_text(json.dumps(state, indent=2), encoding="utf-8")
            else:
                paths["manager_state"].unlink(missing_ok=True)
            summary = {
                "status": status, "completed_games": len(self.games), "scheduled_games": self.total,
                "results": result_stats(self.games, cfg["subject"]["name"]),
                "opponents": {
                    opponent["name"]: result_stats(self.games, cfg["subject"]["name"], opponent["name"])
                    for opponent in cfg["opponents"]
                },
                "rating": self.rating, "rating_error": self.rating_error, "compromised": self.compromised,
                "warnings": self.warning_count, "loki_errors": self.loki_errors, "diagnostics": self.diagnostics,
                "wall_time_s": time.monotonic() - self.started,
                "artifacts": {name: str(path) for name, path in paths.items() if name not in ("trace", "ordo_input")},
            }
            paths["summary"].write_text(json.dumps(summary, indent=2), encoding="utf-8")
            paths["ordo_input"].unlink(missing_ok=True)
            paths["trace"].unlink(missing_ok=True)
            self.exit_code = 0 if status == "complete" and not self.loki_errors and self.rating is not None else (3 if status == "paused" else 1)
            self.query_one("#run_view").add_class("hidden")
            self.query_one("#final_view").remove_class("hidden")
            elo = "unavailable" if self.rating is None else f"{self.rating:.1f}" + (f" ± {self.rating_error:.1f}" if self.rating_error is not None else "")
            self.query_one("#final_summary", Static).update(
                f"{status.upper()} | Games {len(self.games)}/{self.total} | Local CCRL-calibrated Elo {elo}\n"
                f"Warnings {self.warning_count} | Loki errors {len(self.loki_errors)} | "
                f"State {'saved' if status == 'paused' else 'not saved'}\nPress q to exit."
            )
            log = self.query_one("#diagnostics", RichLog)
            if not self.diagnostics:
                log.write("No warnings or errors recorded.")
            for diagnostic in self.diagnostics:
                log.write(diagnostic)
            self.finished = True
            self.finalizing = False

    return GauntletApp()


def make_paths(run_dir: Path) -> dict[str, Path]:
    return {
        "run_dir": run_dir, "pgn": run_dir / "gauntlet.pgn", "game_log": run_dir / "gauntlet.log",
        "loki_errors": run_dir / "loki-errors.log", "manager_state": run_dir / "gauntlet-state.json",
        "fastchess_state": run_dir / "fastchess-state.json", "anchors": run_dir / "anchors.txt",
        "ordo_text": run_dir / "ordo.txt", "ordo_csv": run_dir / "ordo.csv",
        "ordo_input": run_dir / ".ordo-input.pgn", "trace": run_dir / ".fastchess-trace.log",
        "resolved": run_dir / "resolved-workload.json", "summary": run_dir / "summary.json",
    }


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    try:
        local_env = load_env_file(args.env_file.resolve() if args.env_file else None)
        cfg, warnings = load_workload(args.workload.resolve(), local_env)
        validate_paths(cfg)
        resume_state = None
        if args.resume_state:
            try:
                resume_state = json.loads(args.resume_state.resolve().read_text(encoding="utf-8"))
            except (OSError, json.JSONDecodeError) as exc:
                raise ConfigError(f"could not read resume state: {exc}") from exc
            if resume_state.get("schema_version") != 1 or resume_state.get("status") != "paused":
                raise ConfigError("resume state is not a paused gauntlet state")
            if resume_state.get("workload_fingerprint") != workload_fingerprint(cfg):
                raise ConfigError("workload does not match the paused gauntlet")
            run_dir = Path(resume_state["run_dir"]).resolve()
        else:
            root = args.output_dir.resolve() if args.output_dir else Path(__file__).resolve().parent / "results" / safe_name(cfg["name"], "gauntlet")
            run_dir = root / datetime.now().strftime("%Y%m%d-%H%M%S")
        run_dir.mkdir(parents=True, exist_ok=True)
        paths = make_paths(run_dir)
        if resume_state:
            if Path(resume_state["fastchess_state"]).resolve() != paths["fastchess_state"] or not paths["fastchess_state"].is_file():
                raise ConfigError("fastchess state referenced by the resume file is missing")
            try:
                previous = json.loads(paths["resolved"].read_text(encoding="utf-8"))["workload"]
            except (OSError, json.JSONDecodeError, KeyError, TypeError) as exc:
                raise ConfigError(f"could not read the original resolved workload: {exc}") from exc
            update_fastchess_paths(paths["fastchess_state"], previous, cfg)
            if paths["pgn"].is_file():
                with paths["pgn"].open("r+b") as handle:
                    handle.truncate(int(resume_state["pgn_bytes"]))
        write_anchors(cfg, paths["anchors"])
        paths["game_log"].touch(exist_ok=True)
        paths["loki_errors"].touch(exist_ok=True)
        command = build_command(cfg, paths, resume=resume_state is not None)
        paths["resolved"].write_text(json.dumps({"workload": cfg, "command": command}, indent=2), encoding="utf-8")
        textual = load_textual()
    except ConfigError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    result = make_app(textual, cfg, paths, command, warnings, resume_state).run()
    return int(result if result is not None else 0)


if __name__ == "__main__":
    raise SystemExit(main())
