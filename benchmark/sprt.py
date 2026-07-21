#!/usr/bin/env python3

# Loki, a UCI-compliant chess playing software
# Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
#
# Loki is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

"""Build the working tree and a pinned baseline, then run a Fastchess SPRT."""

from __future__ import annotations

import argparse
import asyncio
import hashlib
import json
import math
import os
import re
import shutil
import subprocess
import sys
import time
from datetime import datetime
from pathlib import Path
from typing import Any

from match_runner import (
    COMMON_MATCH_CSS,
    ENGINE_FAILURE_WORDS,
    FINISH_RE,
    START_RE,
    ConfigError,
    compose_match_view,
    configure_game_table,
    engine_args,
    executable_exists,
    expand_env,
    file_sha256,
    load_env_file,
    load_textual,
    parse_games,
    path_value,
    pgn_blocks,
    pgn_game_records,
    refresh_game_table,
    result_stats,
    safe_name,
    stop_process_tree,
)


NUMBER = r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)"
LEGACY_SPRT_RE = re.compile(
    rf"LLR:\s*(?P<llr>{NUMBER})\s*\(\s*(?P<lower>{NUMBER})\s*,\s*(?P<upper>{NUMBER})\s*\)"
    rf"\s*<\s*(?P<elo0>{NUMBER})\s*,\s*(?P<elo1>{NUMBER})\s*>",
    re.I,
)
PENTA_SPRT_RE = re.compile(
    rf"LLR:\s*(?P<llr>{NUMBER})\s*\([^)]*%\)\s*"
    rf"\(\s*(?P<lower>{NUMBER})\s*,\s*(?P<upper>{NUMBER})\s*\)\s*"
    rf"\[\s*(?P<elo0>{NUMBER})\s*,\s*(?P<elo1>{NUMBER})\s*\]",
    re.I,
)
SPRT_RE = re.compile(
    rf"SPRT:\s*llr\s+(?P<llr>{NUMBER}).*?lbound\s+(?P<lower>{NUMBER})\s*,\s*ubound\s+(?P<upper>{NUMBER})",
    re.I,
)
PENTA_RE = re.compile(
    r"Ptnml\(0-2\):\s*\[?\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\]?",
    re.I,
)
BRACKET_PENTA_RE = re.compile(r"^\s*\[(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\]\s*$")
TOTAL_SCORE_RE = re.compile(r"Total:\s*(\d+)\s+W:\s*(\d+)\s+L:\s*(\d+)\s+D:\s*(\d+)", re.I)
SCORE_RE = re.compile(r"Score of .+? vs .+?:\s*(\d+)\s*-\s*(\d+)\s*-\s*(\d+)", re.I)
ELO_RE = re.compile(
    rf"Elo difference:\s*(?P<difference>{NUMBER})\s*\+/-\s*(?P<error>{NUMBER})\s*,\s*"
    rf"LOS:\s*(?P<los>{NUMBER})\s*%\s*,\s*DrawRatio:\s*(?P<draw_ratio>{NUMBER})\s*%",
    re.I,
)
FASTCHESS_ELO_RE = re.compile(
    rf"Elo:\s*(?P<elo>{NUMBER})\s*\+/-\s*(?P<elo_error>{NUMBER})\s*,\s*"
    rf"nElo:\s*(?P<normalized>{NUMBER})\s*\+/-\s*(?P<normalized_error>{NUMBER})",
    re.I,
)
FASTCHESS_LOS_RE = re.compile(
    rf"LOS:\s*(?P<los>{NUMBER})\s*%\s*,\s*DrawRatio:\s*(?P<draw_ratio>{NUMBER})\s*%",
    re.I,
)
REPORT_RE = re.compile(
    r"^(?:-{10,}|Results of |Elo:|LOS:|Games:|Score of |"
    r"\.\.\.\s+.+playing (?:White|Black):|\.\.\.\s+White vs Black:|"
    r"Elo difference:|SPRT:|LLR:|Ptnml\(0-2\):|\[\d+\s*,)",
    re.I,
)
GAME_REFERENCE_RE = re.compile(r"\bgame(?:\s+#|\s+)?(\d+)\b", re.I)
LOG_PREFIX_RE = re.compile(r"^\[[^]]+\]\s+")

MODE_DEFAULTS = {
    "gainer": (0.0, 10.0),
    "non_regression": (-10.0, 0.0),
}
CANDIDATE_NAME = "Loki-candidate"
BASELINE_NAME = "Loki-baseline"


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build Loki's working tree and run a resumable SPRT against a Git revision.")
    parser.add_argument("--workload", required=True, type=Path, help="JSON SPRT workload definition.")
    parser.add_argument("--baseline", help="Baseline Git revision (default: local HEAD).")
    parser.add_argument("--env-file", type=Path, help="Local JSON values for ${NAME} placeholders.")
    parser.add_argument("--resume-state", type=Path, help="sprt-state.json from a paused run.")
    parser.add_argument("--output-dir", type=Path, help="Output directory for a new run.")
    return parser.parse_args(argv)


def _number(value: Any, label: str) -> float:
    if not isinstance(value, (int, float)) or isinstance(value, bool) or not math.isfinite(float(value)):
        raise ConfigError(f"{label} must be a finite number")
    return float(value)


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
    for key in ("name", "tools", "sprt", "match"):
        if key not in cfg:
            raise ConfigError(f"missing workload field: {key}")
    if not isinstance(cfg["name"], str) or not cfg["name"].strip():
        raise ConfigError("name must be a non-empty string")

    base = path.resolve().parent
    tools = cfg["tools"]
    if not isinstance(tools, dict) or not isinstance(tools.get("fastchess"), str) or not tools["fastchess"]:
        raise ConfigError("tools.fastchess must be an executable path or command")
    if any(sep in tools["fastchess"] for sep in ("/", "\\")):
        tools["fastchess"] = path_value(base, tools["fastchess"])

    sprt = cfg["sprt"]
    if not isinstance(sprt, dict):
        raise ConfigError("sprt must be an object")
    mode = sprt.get("mode")
    if mode not in MODE_DEFAULTS:
        raise ConfigError("sprt.mode must be gainer or non_regression")
    default_elo0, default_elo1 = MODE_DEFAULTS[mode]
    sprt["elo0"] = _number(sprt.get("elo0", default_elo0), "sprt.elo0")
    sprt["elo1"] = _number(sprt.get("elo1", default_elo1), "sprt.elo1")
    sprt["alpha"] = _number(sprt.get("alpha", 0.05), "sprt.alpha")
    sprt["beta"] = _number(sprt.get("beta", 0.05), "sprt.beta")
    sprt["model"] = sprt.get("model", "normalized")
    if sprt["elo0"] >= sprt["elo1"]:
        raise ConfigError("sprt.elo0 must be less than sprt.elo1")
    if not 0 < sprt["alpha"] < 1 or not 0 < sprt["beta"] < 1:
        raise ConfigError("sprt.alpha and sprt.beta must be between 0 and 1")
    if sprt["alpha"] + sprt["beta"] >= 1:
        raise ConfigError("sprt.alpha plus sprt.beta must be less than 1")
    if sprt["model"] not in ("normalized", "logistic"):
        raise ConfigError("sprt.model must be normalized or logistic; Fastchess cannot use bayesian SPRT with pentanomial reporting")

    engine = cfg.setdefault("engine", {})
    if not isinstance(engine, dict):
        raise ConfigError("engine must be an object")
    engine.setdefault("args", [])
    engine.setdefault("uci_options", {})
    if not isinstance(engine["args"], list) or not all(isinstance(arg, str) for arg in engine["args"]):
        raise ConfigError("engine.args must be a list of strings")
    if not isinstance(engine["uci_options"], dict):
        raise ConfigError("engine.uci_options must be an object")

    match = cfg["match"]
    if not isinstance(match, dict):
        raise ConfigError("match must be an object")
    match.setdefault("rounds", 100000)
    match.setdefault("concurrency", 1)
    match.setdefault("time_margin_ms", 1000)
    match.setdefault("recover", False)
    if not isinstance(match["rounds"], int) or isinstance(match["rounds"], bool) or match["rounds"] < 1:
        raise ConfigError("match.rounds must be a positive integer")
    if not isinstance(match["concurrency"], int) or isinstance(match["concurrency"], bool) or match["concurrency"] < 1:
        raise ConfigError("match.concurrency must be a positive integer")
    if not isinstance(match["time_margin_ms"], int) or isinstance(match["time_margin_ms"], bool) or match["time_margin_ms"] < 0:
        raise ConfigError("match.time_margin_ms must be a non-negative integer")
    if not isinstance(match["recover"], bool):
        raise ConfigError("match.recover must be a boolean")
    if not isinstance(match.get("time_control"), str) or not match["time_control"]:
        raise ConfigError("match.time_control is required")
    openings = match.get("openings")
    if not isinstance(openings, dict) or not isinstance(openings.get("path"), str) or not openings["path"]:
        raise ConfigError("match.openings.path is required")
    openings["path"] = path_value(base, openings["path"])
    openings.setdefault("format", Path(openings["path"]).suffix.lstrip(".").lower())
    openings.setdefault("order", "random")
    if openings["format"] not in ("epd", "pgn"):
        raise ConfigError("match.openings.format must be epd or pgn")
    if openings["order"] not in ("sequential", "random"):
        raise ConfigError("match.openings.order must be sequential or random")
    return cfg, []


def validate_paths(cfg: dict[str, Any]) -> None:
    if not executable_exists(cfg["tools"]["fastchess"]):
        raise ConfigError(f"fastchess executable does not exist: {cfg['tools']['fastchess']}")
    if not Path(cfg["match"]["openings"]["path"]).is_file():
        raise ConfigError(f"opening suite does not exist: {cfg['match']['openings']['path']}")


def workload_fingerprint(cfg: dict[str, Any]) -> str:
    stable = json.loads(json.dumps(cfg))
    stable["tools"]["fastchess"] = ""
    stable["match"]["openings"]["path"] = ""
    return hashlib.sha256(json.dumps(stable, sort_keys=True, separators=(",", ":")).encode()).hexdigest()


def parse_sprt_line(line: str) -> dict[str, Any]:
    result: dict[str, Any] = {}
    if match := SPRT_RE.search(line):
        result.update({name: float(match.group(name)) for name in ("llr", "lower", "upper")})
    elif match := PENTA_SPRT_RE.search(line):
        result.update({name: float(match.group(name)) for name in ("llr", "lower", "upper", "elo0", "elo1")})
    elif match := LEGACY_SPRT_RE.search(line):
        result.update({name: float(match.group(name)) for name in ("llr", "lower", "upper", "elo0", "elo1")})
    if match := PENTA_RE.search(line) or BRACKET_PENTA_RE.search(line):
        result["pentanomial"] = [int(value) for value in match.groups()]
    if match := TOTAL_SCORE_RE.search(line):
        total, wins, losses, draws = (int(value) for value in match.groups())
        result["score"] = {"total": total, "wins": wins, "losses": losses, "draws": draws}
    elif match := SCORE_RE.search(line):
        wins, losses, draws = (int(value) for value in match.groups())
        result["score"] = {"total": wins + losses + draws, "wins": wins, "losses": losses, "draws": draws}
    if "H1 was accepted" in line:
        result["decision"] = "h1_accepted"
    elif "H0 was accepted" in line:
        result["decision"] = "h0_accepted"
    if match := ELO_RE.search(line):
        result["elo"] = {name: float(match.group(name)) for name in ("difference", "error", "los", "draw_ratio")}
    elif match := FASTCHESS_ELO_RE.search(line):
        result["elo"] = {name: float(match.group(name)) for name in ("elo", "elo_error", "normalized", "normalized_error")}
    elif match := FASTCHESS_LOS_RE.search(line):
        result["elo"] = {name: float(match.group(name)) for name in ("los", "draw_ratio")}
    return result


def is_result_report_line(line: str) -> bool:
    return REPORT_RE.search(line.strip()) is not None


def collect_report_line(report: list[str], line: str) -> list[str]:
    """Collect one native Fastchess report without losing its closing separator."""
    normalized = line.lstrip().lower()
    is_separator = normalized.startswith("----------")
    is_heading = normalized.startswith("results of ") or normalized.startswith("score of ")
    has_heading = any(
        item.lstrip().lower().startswith(("results of ", "score of ")) for item in report
    )

    if is_separator:
        if not report:
            return [line]
        if report[-1].lstrip().startswith("----------"):
            return report
        return [*report, line]

    if is_heading and has_heading:
        separator = next(
            (item for item in reversed(report) if item.lstrip().startswith("----------")), None
        )
        report = [separator] if separator is not None else []
    elif normalized.startswith("ptnml(0-2):") and not has_heading and any(
        item.lstrip().lower().startswith("llr:") for item in report
    ):
        report = []
    elif normalized.startswith("llr:") and (
        not report or report[0].lstrip().lower().startswith("llr:")
    ):
        report = []
    return [*report, line]


def latest_report_from_log(path: Path) -> list[str]:
    report: list[str] = []
    if not path.is_file():
        return report
    for logged_line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = LOG_PREFIX_RE.sub("", logged_line, count=1)
        if is_result_report_line(line):
            report = collect_report_line(report, line)
    return report


def llr_thresholds(alpha: float, beta: float) -> tuple[float, float]:
    return math.log(beta / (1.0 - alpha)), math.log((1.0 - beta) / alpha)


def classify_outcome(llr: float | None, lower: float, upper: float, games: int, maximum: int, compromised: bool = False) -> str:
    if compromised:
        return "compromised"
    if llr is not None and llr >= upper:
        return "h1_accepted"
    if llr is not None and llr <= lower:
        return "h0_accepted"
    return "inconclusive" if games >= maximum else "failed"


def exit_code_for(outcome: str) -> int:
    return {"h1_accepted": 0, "h0_accepted": 1, "paused": 3, "inconclusive": 4}.get(outcome, 5)


def git_output(repo: Path, *args: str) -> str:
    proc = subprocess.run(["git", *args], cwd=repo, capture_output=True, text=True, encoding="utf-8", errors="replace", check=False)
    if proc.returncode:
        raise ConfigError(proc.stderr.strip() or proc.stdout.strip() or f"git {' '.join(args)} failed")
    return proc.stdout.strip()


def resolve_revision(repo: Path, revision: str) -> str:
    return git_output(repo, "rev-parse", "--verify", f"{revision}^{{commit}}")


def working_tree_manifest(repo: Path) -> dict[str, Any]:
    head = resolve_revision(repo, "HEAD")
    status = git_output(repo, "status", "--porcelain=v1", "--untracked-files=all")
    diff = subprocess.run(["git", "diff", "--binary", "HEAD", "--", "."], cwd=repo, capture_output=True, check=False).stdout
    untracked_raw = subprocess.run(
        ["git", "ls-files", "--others", "--exclude-standard", "-z"], cwd=repo, capture_output=True, check=False
    ).stdout
    untracked: dict[str, str] = {}
    for raw in untracked_raw.split(b"\0"):
        if not raw:
            continue
        relative = raw.decode("utf-8", errors="surrogateescape")
        file_path = repo / relative
        if file_path.is_file():
            untracked[relative] = file_sha256(file_path)
    identity = {"head": head, "status": status, "tracked_diff_sha256": hashlib.sha256(diff).hexdigest(), "untracked": untracked}
    identity["fingerprint"] = hashlib.sha256(json.dumps(identity, sort_keys=True).encode()).hexdigest()
    identity["dirty"] = bool(status)
    return identity


def engine_definition(name: str, command: Path, cfg: dict[str, Any], working_directory: Path) -> dict[str, Any]:
    return {
        "name": name, "command": str(command.resolve()), "working_directory": str(working_directory.resolve()),
        "args": list(cfg["engine"]["args"]), "uci_options": dict(cfg["engine"]["uci_options"]),
    }


def validate_archived_engines(candidate: Path, baseline: Path) -> tuple[str, str]:
    if not candidate.is_file():
        raise ConfigError(f"candidate build did not produce {candidate}")
    if not baseline.is_file():
        raise ConfigError(f"baseline build did not produce {baseline}")
    candidate_hash, baseline_hash = file_sha256(candidate), file_sha256(baseline)
    if candidate_hash == baseline_hash:
        raise ConfigError("candidate and baseline executables are identical")
    return candidate_hash, baseline_hash


def build_fastchess_command(cfg: dict[str, Any], paths: dict[str, Path], resume: bool = False) -> list[str]:
    if resume:
        return [
            cfg["tools"]["fastchess"], "-config", f"file={paths['fastchess_state']}", "stats=true",
            "-report", "penta=true",
        ]
    candidate = engine_definition(CANDIDATE_NAME, paths["candidate_engine"], cfg, paths["engines_dir"])
    baseline = engine_definition(BASELINE_NAME, paths["baseline_engine"], cfg, paths["engines_dir"])
    match, openings, sprt = cfg["match"], cfg["match"]["openings"], cfg["sprt"]
    command = [cfg["tools"]["fastchess"]]
    for engine in (candidate, baseline):
        command.extend(["-engine", *engine_args(engine)])
    opening_args = [f"file={openings['path']}", f"format={openings['format']}", f"order={openings['order']}"]
    if openings.get("plies") is not None:
        opening_args.append(f"plies={openings['plies']}")
    command.extend([
        "-rounds", str(match["rounds"]), "-games", "2", "-repeat",
        "-concurrency", str(match["concurrency"]),
        "-each", f"tc={match['time_control']}", f"timemargin={match['time_margin_ms']}",
        "-openings", *opening_args,
        "-sprt", f"elo0={sprt['elo0']:g}", f"elo1={sprt['elo1']:g}",
        f"alpha={sprt['alpha']:g}", f"beta={sprt['beta']:g}", f"model={sprt['model']}",
        "-report", "penta=true",
        "-pgnout", f"file={paths['pgn']}", "notation=san", "append=false",
        "-log", f"file={paths['trace']}", "level=trace", "append=false", "realtime=true", "engine=true",
        "-config", f"outname={paths['fastchess_state']}", "stats=true", "-autosaveinterval", "1",
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


def make_paths(run_dir: Path) -> dict[str, Path]:
    engines = run_dir / "engines"
    suffix = ".exe" if os.name == "nt" else ""
    return {
        "run_dir": run_dir, "engines_dir": engines,
        "candidate_engine": engines / f"Loki-candidate{suffix}", "baseline_engine": engines / f"Loki-baseline{suffix}",
        "baseline_worktree": run_dir / ".baseline-worktree", "pgn": run_dir / "sprt.pgn",
        "game_log": run_dir / "sprt.log", "engine_errors": run_dir / "engine-errors.log",
        "build_log": run_dir / "build.log", "manager_state": run_dir / "sprt-state.json",
        "fastchess_state": run_dir / "fastchess-state.json", "trace": run_dir / ".fastchess-trace.log",
        "resolved": run_dir / "resolved-workload.json", "metadata": run_dir / "build-metadata.json",
        "summary": run_dir / "summary.json", "result_text": run_dir / "result.txt",
    }


def _source_engine(source: Path) -> Path:
    return source / "build" / "release" / "bin" / ("Loki.exe" if os.name == "nt" else "Loki")


def make_app(
    textual: dict[str, Any], cfg: dict[str, Any], paths: dict[str, Path], repo: Path,
    baseline_ref: str, warnings: list[str], resume_state: dict[str, Any] | None,
):
    App, ComposeResult = textual["App"], textual["ComposeResult"]
    DataTable, ProgressBar, RichLog, Static = textual["DataTable"], textual["ProgressBar"], textual["RichLog"], textual["Static"]

    class SprtApp(App):
        CSS = COMMON_MATCH_CSS
        BINDINGS = [("p", "pause", "Pause/save"), ("q", "quit", "Quit"), ("ctrl+c", "quit", "Cancel/Quit")]

        def __init__(self) -> None:
            super().__init__()
            self.proc: asyncio.subprocess.Process | None = None
            self.run_task: asyncio.Task[None] | None = None
            self.trace_task: asyncio.Task[None] | None = None
            self.stop_reason: str | None = None
            self.phase = "resume" if resume_state else "setup"
            self.finished = self.finalizing = False
            self.started = time.monotonic()
            self.games: list[dict[str, str]] = []
            self.diagnostics = list(warnings)
            self.warning_count = len(warnings)
            self.engine_errors: list[dict[str, Any]] = []
            self.seen_errors: set[str] = set()
            self.active_games: dict[int, tuple[str, str]] = {}
            self.compromised = False
            self.llr: float | None = None
            self.reported_decision: str | None = None
            self.lower, self.upper = llr_thresholds(cfg["sprt"]["alpha"], cfg["sprt"]["beta"])
            self.pentanomial = [0, 0, 0, 0, 0]
            self.elo: dict[str, float] | None = None
            self.latest_report: list[str] = []
            self.baseline_sha: str | None = resume_state.get("baseline_sha") if resume_state else None
            self.build_metadata: dict[str, Any] = {}
            self.exit_code = 2
            self.total = cfg["match"]["rounds"] * 2

        def compose(self) -> ComposeResult:
            yield from compose_match_view(textual, self.total, "SPRT", "sprt")

        async def on_mount(self) -> None:
            self.title = "Loki SPRT"
            table = self.query_one("#sprt", DataTable)
            table.add_column("Field", key="field")
            table.add_column("Value", key="value")
            for key in ("candidate", "baseline", "hypotheses", "llr", "pentanomial", "score", "decision"):
                table.add_row(key.replace("_", " ").title(), "-", key=key)
            configure_game_table(self.query_one("#games", DataTable))
            self._refresh_games()
            for warning in warnings:
                self._log(f"[yellow]Warning: {warning}[/yellow]")
            self._log("Resuming archived SPRT" if resume_state else f"Preparing candidate and baseline {baseline_ref}")
            self.run_task = asyncio.create_task(self._orchestrate())

        def _log(self, message: str) -> None:
            self.query_one("#event_log", RichLog).write(f"[{time.strftime('%H:%M:%S')}] {message}")

        def _log_plain(self, message: str) -> None:
            self._log(message.replace("[", r"\["))

        def _record_report_line(self, line: str) -> None:
            self.latest_report = collect_report_line(self.latest_report, line)
            with paths["game_log"].open("a", encoding="utf-8") as handle:
                handle.write(f"[{datetime.now().astimezone().isoformat(timespec='seconds')}] {line}\n")
            self._log_plain(line)

        def _decision(self) -> str:
            if self.compromised:
                return "compromised"
            if self.reported_decision is not None:
                return self.reported_decision
            if self.llr is not None and self.llr >= self.upper:
                return "h1_accepted"
            if self.llr is not None and self.llr <= self.lower:
                return "h0_accepted"
            return "running" if self.phase == "match" else self.phase

        def _elo_summary(self) -> str:
            if not self.elo:
                return "unavailable"
            if "normalized" in self.elo:
                return (
                    f"{self.elo['elo']:.2f} +/- {self.elo['elo_error']:.2f}, "
                    f"nElo {self.elo['normalized']:.2f} +/- {self.elo['normalized_error']:.2f}"
                )
            if "difference" in self.elo:
                return f"{self.elo['difference']:.1f} +/- {self.elo['error']:.1f}"
            return "unavailable"

        def _render_summary(self) -> None:
            stats = result_stats(self.games, CANDIDATE_NAME)
            llr = "pending" if self.llr is None else f"{self.llr:.3f} ({self.lower:.3f}, {self.upper:.3f})"
            self.query_one("#summary", Static).update(
                f"Stage {self.phase} | Games {len(self.games)}/{self.total} | W/D/L {stats['wins']}/{stats['draws']}/{stats['losses']} | "
                f"Warnings {self.warning_count} | Engine errors {len(self.engine_errors)}\n"
                f"LLR {llr} | Decision {self._decision()} | Wall {time.monotonic() - self.started:.1f}s | p pause/save | q cancel"
            )

        def _refresh_details(self) -> None:
            stats = result_stats(self.games, CANDIDATE_NAME)
            score = (stats["wins"] + stats["draws"] / 2) / stats["played"] if stats["played"] else None
            values = {
                "candidate": self.build_metadata.get("candidate", {}).get("label", "working tree"),
                "baseline": self.baseline_sha or baseline_ref,
                "hypotheses": f"<{cfg['sprt']['elo0']:g}, {cfg['sprt']['elo1']:g}> {cfg['sprt']['model']}",
                "llr": "pending" if self.llr is None else f"{self.llr:.3f} ({self.lower:.3f}, {self.upper:.3f})",
                "pentanomial": ", ".join(map(str, self.pentanomial)),
                "score": "-" if score is None else f"{score:.2%} ({stats['wins']}/{stats['draws']}/{stats['losses']})",
                "decision": self._decision(),
            }
            table = self.query_one("#sprt", DataTable)
            for key, value in values.items():
                table.update_cell(key, "value", str(value))

        def _refresh_games(self) -> None:
            self.games = parse_games(paths["pgn"])
            self._load_pentanomial_state()
            self.query_one("#progress", ProgressBar).update(progress=min(len(self.games), self.total))
            refresh_game_table(self.query_one("#games", DataTable), self.games)
            self._refresh_details()
            self._render_summary()

        async def _run_logged(self, command: list[str], cwd: Path, label: str) -> None:
            self._log(f"[bold]{label}[/bold]: {subprocess.list2cmdline(command)}")
            with paths["build_log"].open("a", encoding="utf-8") as log:
                log.write(f"\n[{datetime.now().astimezone().isoformat()}] {label}: {subprocess.list2cmdline(command)}\n")
            flags = subprocess.CREATE_NEW_PROCESS_GROUP if os.name == "nt" else 0
            kwargs: dict[str, Any] = {"creationflags": flags} if os.name == "nt" else {"start_new_session": True}
            self.proc = await asyncio.create_subprocess_exec(
                *command, cwd=cwd, stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.STDOUT, **kwargs
            )
            assert self.proc.stdout is not None
            while raw := await self.proc.stdout.readline():
                line = raw.decode(errors="replace").rstrip()
                if line:
                    self._log(line)
                    with paths["build_log"].open("a", encoding="utf-8") as log:
                        log.write(line + "\n")
            code = await self.proc.wait()
            self.proc = None
            if code:
                if self.stop_reason == "canceled":
                    raise asyncio.CancelledError
                raise ConfigError(f"{label} exited with code {code}")

        async def _prepare(self) -> None:
            paths["engines_dir"].mkdir(parents=True, exist_ok=True)
            self._log(f"Resolving baseline revision {baseline_ref}")
            self.baseline_sha = await asyncio.to_thread(resolve_revision, repo, baseline_ref)
            self._log(f"Baseline resolved to {self.baseline_sha}")
            start_manifest = await asyncio.to_thread(working_tree_manifest, repo)
            self.build_metadata["candidate"] = {"label": f"{start_manifest['head'][:12]}{' + working changes' if start_manifest['dirty'] else ''}", **start_manifest}
            self._refresh_details()
            await self._run_logged(["cmake", "--preset", "release"], repo, "Configure candidate")
            await self._run_logged(["cmake", "--build", "--preset", "release", "--target", "Loki"], repo, "Build candidate")
            end_manifest = await asyncio.to_thread(working_tree_manifest, repo)
            if start_manifest["fingerprint"] != end_manifest["fingerprint"]:
                raise ConfigError("working tree changed while the candidate was being built")
            candidate_source = _source_engine(repo)
            if not candidate_source.is_file():
                raise ConfigError(f"candidate build did not produce {candidate_source}")
            shutil.copy2(candidate_source, paths["candidate_engine"])

            self._refresh_details()
            worktree = paths["baseline_worktree"]
            try:
                await self._run_logged(["git", "worktree", "add", "--detach", str(worktree), self.baseline_sha], repo, "Create baseline worktree")
                await self._run_logged(["cmake", "--preset", "release"], worktree, "Configure baseline")
                await self._run_logged(["cmake", "--build", "--preset", "release", "--target", "Loki"], worktree, "Build baseline")
                baseline_source = _source_engine(worktree)
                if not baseline_source.is_file():
                    raise ConfigError(f"baseline build did not produce {baseline_source}")
                shutil.copy2(baseline_source, paths["baseline_engine"])
            finally:
                if worktree.exists():
                    try:
                        await self._run_logged(["git", "worktree", "remove", "--force", str(worktree)], repo, "Remove baseline worktree")
                    except (ConfigError, asyncio.CancelledError) as exc:
                        self._record_diagnostic(f"could not remove temporary baseline worktree: {exc}")

            candidate_hash, baseline_hash = validate_archived_engines(paths["candidate_engine"], paths["baseline_engine"])
            self.build_metadata["candidate"].update({"executable": str(paths["candidate_engine"]), "sha256": candidate_hash})
            self.build_metadata["baseline"] = {
                "label": self.baseline_sha[:12], "revision": baseline_ref, "commit": self.baseline_sha,
                "executable": str(paths["baseline_engine"]), "sha256": baseline_hash,
            }
            self.build_metadata["commands"] = [
                ["cmake", "--preset", "release"],
                ["cmake", "--build", "--preset", "release", "--target", "Loki"],
            ]
            paths["metadata"].write_text(json.dumps(self.build_metadata, indent=2), encoding="utf-8")

        def _verify_resume(self) -> None:
            assert resume_state is not None
            try:
                metadata = json.loads(paths["metadata"].read_text(encoding="utf-8"))
            except (OSError, json.JSONDecodeError) as exc:
                raise ConfigError(f"could not read build metadata: {exc}") from exc
            for side, path in (("candidate", paths["candidate_engine"]), ("baseline", paths["baseline_engine"])):
                if not path.is_file() or file_sha256(path) != metadata[side]["sha256"]:
                    raise ConfigError(f"archived {side} executable is missing or has changed")
            if not paths["fastchess_state"].is_file():
                raise ConfigError("Fastchess resume state is missing")
            self.build_metadata = metadata
            self.baseline_sha = metadata["baseline"]["commit"]
            if paths["pgn"].is_file():
                with paths["pgn"].open("r+b") as handle:
                    handle.truncate(int(resume_state["pgn_bytes"]))

        async def _orchestrate(self) -> None:
            try:
                if not resume_state:
                    paths["resolved"].write_text(json.dumps({
                        "workload": cfg, "baseline_ref": baseline_ref, "baseline_sha": None,
                        "command": None, "build_metadata": None,
                    }, indent=2), encoding="utf-8")
                if resume_state:
                    await asyncio.to_thread(self._verify_resume)
                else:
                    await self._prepare()
                if self.stop_reason == "canceled":
                    await self._finish("canceled")
                    return
                command = build_fastchess_command(cfg, paths, resume=resume_state is not None)
                if resume_state:
                    try:
                        resolved = json.loads(paths["resolved"].read_text(encoding="utf-8"))
                    except (OSError, json.JSONDecodeError) as exc:
                        raise ConfigError(f"could not read the original resolved workload: {exc}") from exc
                    resolved["resume_command"] = command
                else:
                    resolved = {
                        "workload": cfg, "baseline_ref": baseline_ref, "baseline_sha": self.baseline_sha,
                        "command": command, "build_metadata": self.build_metadata,
                    }
                paths["resolved"].write_text(json.dumps(resolved, indent=2), encoding="utf-8")
                self.phase = "match"
                self._log("Starting Fastchess SPRT" if not resume_state else "Continuing Fastchess SPRT")
                self._refresh_games()
                await self._run_fastchess(command)
            except asyncio.CancelledError:
                await self._finish("canceled")
            except ConfigError as exc:
                self._record_diagnostic(str(exc), error=True)
                await self._finish("canceled" if self.stop_reason == "canceled" else "setup_error")
            except Exception as exc:
                self._record_diagnostic(f"unexpected failure: {exc}", error=True)
                await self._finish("failed")

        async def _run_fastchess(self, command: list[str]) -> None:
            flags = subprocess.CREATE_NEW_PROCESS_GROUP if os.name == "nt" else 0
            kwargs: dict[str, Any] = {"creationflags": flags} if os.name == "nt" else {"start_new_session": True}
            try:
                self.proc = await asyncio.create_subprocess_exec(
                    *command, stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE, **kwargs
                )
                assert self.proc.stdout is not None and self.proc.stderr is not None
                self.trace_task = asyncio.create_task(self._tail_trace())
                readers = [
                    asyncio.create_task(self._read_stream(self.proc.stdout, "stdout")),
                    asyncio.create_task(self._read_stream(self.proc.stderr, "stderr")),
                ]
                code = await self.proc.wait()
                await asyncio.gather(*readers, return_exceptions=True)
                if self.trace_task:
                    await self.trace_task
                self._refresh_games()
                if self.stop_reason is None and code:
                    self.stop_reason = "failed"
                    self._record_diagnostic(f"Fastchess exited with code {code}", error=True)
            except OSError as exc:
                self.stop_reason = "failed"
                self._record_diagnostic(f"could not start Fastchess: {exc}", error=True)
            await self._finish(self.stop_reason)

        async def _read_stream(self, stream: asyncio.StreamReader, source: str) -> None:
            while raw := await stream.readline():
                line = raw.decode(errors="replace").rstrip()
                started = START_RE.search(line)
                finished = FINISH_RE.search(line)
                if started:
                    self.active_games[int(started.group(1))] = (started.group(3), started.group(4))
                parsed = parse_sprt_line(line)
                if "llr" in parsed:
                    self.llr, self.lower, self.upper = parsed["llr"], parsed["lower"], parsed["upper"]
                    self._refresh_details()
                    self._render_summary()
                if "pentanomial" in parsed:
                    self.pentanomial = parsed["pentanomial"]
                    self._refresh_details()
                if "decision" in parsed:
                    self.reported_decision = parsed["decision"]
                    self._refresh_details()
                    self._render_summary()
                if "elo" in parsed:
                    self.elo = {**(self.elo or {}), **parsed["elo"]}
                if is_result_report_line(line):
                    self._record_report_line(line)
                if started or finished:
                    with paths["game_log"].open("a", encoding="utf-8") as handle:
                        handle.write(f"[{datetime.now().astimezone().isoformat(timespec='seconds')}] {line}\n")
                    self._log(line)
                if finished:
                    await asyncio.sleep(0.05)
                    self._refresh_games()
                lower = line.lower()
                if source == "stderr" or any(
                    word in lower for word in ("warning", "error", "fatal", *ENGINE_FAILURE_WORDS)
                ):
                    self._classify_line(line)
                if finished:
                    self.active_games.pop(int(finished.group(1)), None)

        def _classify_line(self, line: str) -> None:
            normalized = line.strip()
            if not normalized or normalized in self.seen_errors:
                return
            lower = normalized.lower()
            is_failure = any(word in lower for word in ENGINE_FAILURE_WORDS)
            finish = FINISH_RE.search(normalized)
            game_reference = finish or GAME_REFERENCE_RE.search(normalized)
            game_number = int(game_reference.group(1)) if game_reference else None
            if finish and is_failure:
                white, black, result = finish.group(2), finish.group(3), finish.group(4)
                engine = white if result == "0-1" else black if result == "1-0" else None
            else:
                mentioned = [name for name in (CANDIDATE_NAME, BASELINE_NAME) if name.lower() in lower]
                engine = mentioned[0] if len(mentioned) == 1 else None
            if engine is None:
                mentioned = [name for name in (CANDIDATE_NAME, BASELINE_NAME) if name.lower() in lower]
                engine = mentioned[0] if len(mentioned) == 1 else None
            if is_failure and engine and game_number is None:
                active = [number for number, players in self.active_games.items() if engine in players]
                if len(active) == 1:
                    game_number = active[0]
            if is_failure and engine:
                self.seen_errors.add(normalized)
                self.compromised = True
                record = {
                    "timestamp": datetime.now().astimezone().isoformat(timespec="seconds"),
                    "engine": engine,
                    "message": normalized,
                    "game_number": game_number,
                }
                self.engine_errors.append(record)
                with paths["engine_errors"].open("a", encoding="utf-8") as handle:
                    handle.write(f"[{record['timestamp']}] [{engine}] {normalized}\n")
                self._record_diagnostic(f"{engine}: {normalized}", error=True)
                if not cfg["match"]["recover"] and self.stop_reason is None:
                    self.stop_reason = "failed"
                    asyncio.create_task(stop_process_tree(self.proc))
            elif any(word in lower for word in ("warning", "error", "fatal", "failed", "illegal", "timeout", "forfeit")):
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

        def _record_diagnostic(self, message: str, error: bool = False) -> None:
            self.diagnostics.append(message)
            if not error:
                self.warning_count += 1
            if self.is_mounted:
                self._log(f"[{'red' if error else 'yellow'}]{'Error' if error else 'Warning'}: {message}[/]")
                self._render_summary()

        def _write_engine_error_log(self) -> None:
            games: dict[int, dict[str, Any]] = {}
            for game in pgn_game_records(paths["pgn"]):
                headers, round_number = game["headers"], game["round_number"]
                if round_number is not None:
                    if headers.get("White") == CANDIDATE_NAME and headers.get("Black") == BASELINE_NAME:
                        game_number = round_number * 2 - 1
                    elif headers.get("White") == BASELINE_NAME and headers.get("Black") == CANDIDATE_NAME:
                        game_number = round_number * 2
                    else:
                        game_number = game["position"]
                else:
                    game_number = game["position"]
                games[game_number] = game
            sections: list[str] = []
            for index, error in enumerate(self.engine_errors, 1):
                game_number = error.get("game_number")
                section = [
                    f"=== Engine error {index} ===",
                    f"Timestamp: {error['timestamp']}",
                    f"Engine: {error['engine']}",
                    f"Game: {game_number if game_number is not None else 'unidentified'}",
                    f"Message: {error['message']}",
                    "",
                ]
                game = games.get(game_number)
                if game:
                    section.extend(("PGN:", game["pgn"]))
                else:
                    section.append("PGN: unavailable (Fastchess did not identify a completed game for this error).")
                sections.append("\n".join(section))
            paths["engine_errors"].write_text(
                "\n\n".join(sections) + ("\n" if sections else ""), encoding="utf-8"
            )

        async def action_pause(self) -> None:
            if self.finished or self.finalizing:
                return
            if self.phase != "match":
                self._log("Pause is available after Fastchess starts.")
                return
            self.stop_reason = "paused"
            self._log("Pause requested; stopping active games and saving state...")
            await stop_process_tree(self.proc)

        async def action_quit(self) -> None:
            if self.finished:
                self.exit(self.exit_code)
                return
            if self.finalizing:
                return
            self.stop_reason = "canceled"
            self._log("Cancellation requested...")
            await stop_process_tree(self.proc)

        def _load_pentanomial_state(self) -> None:
            try:
                state = json.loads(paths["fastchess_state"].read_text(encoding="utf-8"))
                stats = state.get("stats", {}).get(f"{CANDIDATE_NAME} vs {BASELINE_NAME}", {})
                if stats:
                    self.pentanomial = [
                        int(stats.get("penta_LL", 0)), int(stats.get("penta_LD", 0)),
                        int(stats.get("penta_DD", 0)) + int(stats.get("penta_WL", 0)),
                        int(stats.get("penta_WD", 0)), int(stats.get("penta_WW", 0)),
                    ]
            except (OSError, json.JSONDecodeError, TypeError, ValueError):
                pass

        async def _finish(self, forced: str | None = None) -> None:
            if self.finished or self.finalizing:
                return
            self.finalizing = True
            if paths["pgn"].is_file():
                paths["pgn"].write_bytes(b"".join(pgn_blocks(paths["pgn"])))
            self._write_engine_error_log()
            recovered_report = latest_report_from_log(paths["game_log"])
            if recovered_report:
                self.latest_report = recovered_report
            self._load_pentanomial_state()
            self._refresh_games()
            if forced == "paused" and not paths["fastchess_state"].is_file():
                self._record_diagnostic("Fastchess did not create resumable state", error=True)
                forced = "failed"
            if forced in ("setup_error", "canceled", "failed"):
                outcome = forced
            elif forced == "paused":
                outcome = "paused"
            elif self.compromised:
                outcome = "compromised"
            elif self.reported_decision is not None:
                outcome = self.reported_decision
            else:
                outcome = classify_outcome(self.llr, self.lower, self.upper, len(self.games), self.total, self.compromised)
            if outcome == "paused":
                state = {
                    "schema_version": 1, "status": "paused", "run_dir": str(paths["run_dir"]),
                    "workload_fingerprint": workload_fingerprint(cfg), "baseline_sha": self.baseline_sha,
                    "pgn_bytes": paths["pgn"].stat().st_size if paths["pgn"].exists() else 0,
                    "completed_games": len(self.games),
                }
                paths["manager_state"].write_text(json.dumps(state, indent=2), encoding="utf-8")
            else:
                paths["manager_state"].unlink(missing_ok=True)
            stats = result_stats(self.games, CANDIDATE_NAME)
            summary = {
                "status": outcome, "completed_games": len(self.games), "maximum_games": self.total,
                "results": stats, "pentanomial": self.pentanomial,
                "elo": self.elo, "latest_report": self.latest_report,
                "sprt": {
                    **cfg["sprt"], "llr": self.llr, "lower_bound": self.lower, "upper_bound": self.upper,
                    "reported_decision": self.reported_decision,
                },
                "baseline_sha": self.baseline_sha, "compromised": self.compromised,
                "warnings": self.warning_count, "engine_errors": self.engine_errors, "diagnostics": self.diagnostics,
                "wall_time_s": time.monotonic() - self.started,
                "artifacts": {name: str(path) for name, path in paths.items() if name not in ("trace", "baseline_worktree")},
            }
            paths["summary"].write_text(json.dumps(summary, indent=2), encoding="utf-8")
            if self.latest_report:
                paths["result_text"].write_text("\n".join(self.latest_report) + "\n", encoding="utf-8")
            else:
                paths["result_text"].unlink(missing_ok=True)
            paths["trace"].unlink(missing_ok=True)
            self.phase = outcome
            self.exit_code = 2 if outcome == "setup_error" else exit_code_for(outcome)
            self.query_one("#run_view").add_class("hidden")
            self.query_one("#final_view").remove_class("hidden")
            self.query_one("#final_summary", Static).update(
                f"{outcome.upper()} | Games {len(self.games)}/{self.total} | W/D/L "
                f"{stats['wins']}/{stats['draws']}/{stats['losses']}\n"
                f"LLR {'unavailable' if self.llr is None else f'{self.llr:.3f}'} ({self.lower:.3f}, {self.upper:.3f}) | "
                f"Elo {self._elo_summary()}\n"
                f"Engine errors {len(self.engine_errors)} | State {'saved' if outcome == 'paused' else 'not saved'}\n"
                f"Press q to exit and print the copyable result block."
            )
            log = self.query_one("#diagnostics", RichLog)
            if not self.diagnostics:
                log.write("No warnings or errors recorded.")
            for diagnostic in self.diagnostics:
                log.write(diagnostic)
            self.finished = True
            self.finalizing = False

    return SprtApp()


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    try:
        if args.resume_state and args.baseline is not None:
            raise ConfigError("--baseline cannot be used with --resume-state")
        repo = Path(__file__).resolve().parent.parent
        if not (repo / ".git").exists():
            raise ConfigError(f"repository metadata was not found at {repo}")
        local_env = load_env_file(args.env_file.resolve() if args.env_file else None)
        cfg, warnings = load_workload(args.workload.resolve(), local_env)
        validate_paths(cfg)
        resume_state = None
        baseline_ref = args.baseline or "HEAD"
        if args.resume_state:
            try:
                resume_state = json.loads(args.resume_state.resolve().read_text(encoding="utf-8"))
            except (OSError, json.JSONDecodeError) as exc:
                raise ConfigError(f"could not read resume state: {exc}") from exc
            if resume_state.get("schema_version") != 1 or resume_state.get("status") != "paused":
                raise ConfigError("resume state is not a paused SPRT state")
            if resume_state.get("workload_fingerprint") != workload_fingerprint(cfg):
                raise ConfigError("workload does not match the paused SPRT")
            run_dir = Path(resume_state["run_dir"]).resolve()
            baseline_ref = str(resume_state["baseline_sha"])
        else:
            root = args.output_dir.resolve() if args.output_dir else Path(__file__).resolve().parent / "results" / safe_name(cfg["name"], "sprt")
            run_dir = root / datetime.now().strftime("%Y%m%d-%H%M%S")
        run_dir.mkdir(parents=True, exist_ok=True)
        paths = make_paths(run_dir)
        paths["game_log"].touch(exist_ok=True)
        paths["engine_errors"].touch(exist_ok=True)
        paths["build_log"].touch(exist_ok=True)
        textual = load_textual()
    except ConfigError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    result = make_app(textual, cfg, paths, repo, baseline_ref, warnings, resume_state).run()
    if paths["result_text"].is_file():
        report = paths["result_text"].read_text(encoding="utf-8", errors="replace").rstrip()
        if report:
            print(report)
    return int(result if result is not None else 0)


if __name__ == "__main__":
    raise SystemExit(main())
