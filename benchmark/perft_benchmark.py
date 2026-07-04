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
import json
import os
import re
import subprocess
import sys
import time
from collections import defaultdict
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any


DEPTH_FIELD_RE = re.compile(r"^D(?P<depth>\d+)\s+(?P<nodes>\d+)$")
START_RE = re.compile(r"^<PERFT TEST FOR DEPTH = (?P<depth>\d+)>$")
END_RE = re.compile(r"^</PERFT TEST FOR DEPTH = (?P<depth>\d+)>$")
NODES_RE = re.compile(r"Nodes visited:\s*(?P<nodes>\d+)")
ELAPSED_RE = re.compile(r"Perft test completed after:\s*(?P<elapsed>\d+)ms")
NPS_RE = re.compile(r"Nodes/second:\s*(?P<nps>-?\d+)")


class BenchmarkConfigError(Exception):
    """Raised when the benchmark cannot start."""


class LokiRuntimeError(Exception):
    """Raised when a Loki worker process cannot complete a perft job."""

    def __init__(self, kind: str, message: str) -> None:
        super().__init__(message)
        self.kind = kind
        self.message = message


@dataclass(frozen=True)
class PerftPosition:
    line_number: int
    fen: str
    expected_by_depth: dict[int, int]


@dataclass(frozen=True)
class PerftCase:
    ordinal: int
    position_ordinal: int
    line_number: int
    fen: str
    depth: int
    expected_nodes: int | None


@dataclass(frozen=True)
class PerftJob:
    ordinal: int
    line_number: int
    fen: str
    max_depth: int
    cases: list[PerftCase]
    missing_depths: list[int]


@dataclass
class PerftOutput:
    actual_nodes: int
    elapsed_ms: int
    nps: int


@dataclass
class BenchmarkError:
    fen: str
    depth: int
    kind: str
    message: str
    worker_id: int | None = None
    expected_nodes: int | None = None
    actual_nodes: int | None = None


@dataclass
class BenchmarkWarning:
    fen: str
    line_number: int
    depth: int
    kind: str
    message: str


@dataclass
class BenchmarkResult:
    ordinal: int
    position_ordinal: int
    line_number: int
    fen: str
    depth: int
    expected_nodes: int | None
    status: str
    worker_id: int
    actual_nodes: int | None = None
    elapsed_ms: int | None = None
    nps: int | None = None
    error_kind: str | None = None
    error_message: str | None = None


@dataclass
class BenchmarkSummary:
    total: int
    positions: int
    completed: int = 0
    passed: int = 0
    failed: int = 0
    warnings: int = 0
    runtime_errors: int = 0
    restarts: int = 0
    total_nodes: int = 0
    wall_time_s: float = 0.0
    aggregate_nps: int = 0
    canceled: bool = False


@dataclass
class WorkerSnapshot:
    worker_id: int
    state: str = "starting"
    position_ordinal: int | None = None
    depth: int | None = None
    line_number: int | None = None
    restarts: int = 0


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def default_epd_path() -> Path:
    return repo_root() / "tests" / "perft.epd"


def positive_int(value: str) -> int:
    try:
        parsed = int(value)
    except ValueError as exc:
        raise argparse.ArgumentTypeError("must be an integer") from exc
    if parsed < 1:
        raise argparse.ArgumentTypeError("must be at least 1")
    return parsed


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run Loki perft benchmarks in a Textual TUI.")
    parser.add_argument("--engine", required=True, type=Path, help="Path to a Loki executable built with dev commands.")
    parser.add_argument("--jobs", type=positive_int, default=1, help="Number of Loki worker processes to run in parallel.")
    parser.add_argument("--epd", type=Path, default=default_epd_path(), help="Path to the perft EPD dataset.")
    parser.add_argument("--limit", type=positive_int, help="Maximum number of positions to run.")
    parser.add_argument("--json-out", type=Path, help="Optional path to write full benchmark results as JSON.")
    return parser.parse_args(argv)


def parse_epd(path: Path) -> list[PerftPosition]:
    if not path.is_file():
        raise BenchmarkConfigError(f"EPD file does not exist: {path}")

    positions: list[PerftPosition] = []
    for line_number, raw_line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue

        fields = [field.strip() for field in line.split(";")]
        fen = fields[0]
        if not fen:
            raise BenchmarkConfigError(f"line {line_number}: missing FEN")

        expected_by_depth: dict[int, int] = {}
        for field in fields[1:]:
            if not field:
                continue
            match = DEPTH_FIELD_RE.match(field)
            if not match:
                raise BenchmarkConfigError(f"line {line_number}: malformed EPD depth field '{field}'")
            expected_by_depth[int(match.group("depth"))] = int(match.group("nodes"))

        if not expected_by_depth:
            raise BenchmarkConfigError(f"line {line_number}: no perft depth fields found")

        positions.append(PerftPosition(line_number=line_number, fen=fen, expected_by_depth=expected_by_depth))

    if not positions:
        raise BenchmarkConfigError(f"EPD file contains no positions: {path}")
    return positions


def select_jobs(positions: list[PerftPosition], limit: int | None) -> tuple[list[PerftJob], list[BenchmarkWarning]]:
    jobs: list[PerftJob] = []
    warnings: list[BenchmarkWarning] = []
    case_ordinal = 1

    for position in positions:
        max_depth = max(position.expected_by_depth)
        position_ordinal = len(jobs) + 1
        missing_depths: list[int] = []
        cases: list[PerftCase] = []

        for depth in range(1, max_depth + 1):
            expected = position.expected_by_depth.get(depth)
            if expected is None:
                missing_depths.append(depth)
                warnings.append(
                    BenchmarkWarning(
                        fen=position.fen,
                        line_number=position.line_number,
                        depth=depth,
                        kind="missing_expected_nodes",
                        message=f"line {position.line_number} has no D{depth} expected node count",
                    )
                )

            cases.append(
                PerftCase(
                    ordinal=case_ordinal,
                    position_ordinal=position_ordinal,
                    line_number=position.line_number,
                    fen=position.fen,
                    depth=depth,
                    expected_nodes=expected,
                )
            )
            case_ordinal += 1

        jobs.append(
            PerftJob(
                ordinal=position_ordinal,
                line_number=position.line_number,
                fen=position.fen,
                max_depth=max_depth,
                cases=cases,
                missing_depths=missing_depths,
            )
        )
        if limit is not None and len(jobs) >= limit:
            break

    if not jobs:
        raise BenchmarkConfigError("no positions selected from the EPD file")
    return jobs, warnings


def parse_perft_output(lines: list[str], expected_depth: int) -> PerftOutput:
    start_seen = False
    end_seen = False
    for line in lines:
        start_match = START_RE.match(line)
        if start_match:
            start_seen = int(start_match.group("depth")) == expected_depth
            continue
        end_match = END_RE.match(line)
        if end_match:
            end_seen = int(end_match.group("depth")) == expected_depth

    if not start_seen:
        raise LokiRuntimeError("missing_perft_start", f"missing perft start marker for depth {expected_depth}")
    if not end_seen:
        raise LokiRuntimeError("missing_perft_end", f"missing perft end marker for depth {expected_depth}")

    joined = "\n".join(lines)
    nodes_match = NODES_RE.search(joined)
    elapsed_match = ELAPSED_RE.search(joined)
    nps_match = NPS_RE.search(joined)

    missing = []
    if nodes_match is None:
        missing.append("Nodes visited")
    if elapsed_match is None:
        missing.append("elapsed time")
    if nps_match is None:
        missing.append("Nodes/second")
    if missing:
        raise LokiRuntimeError("malformed_perft_output", f"missing fields: {', '.join(missing)}")

    return PerftOutput(
        actual_nodes=int(nodes_match.group("nodes")),
        elapsed_ms=int(elapsed_match.group("elapsed")),
        nps=int(nps_match.group("nps")),
    )


def validate_engine_path(path: Path) -> None:
    if not path.is_file():
        raise BenchmarkConfigError(f"engine executable does not exist: {path}")
    if os.name != "nt" and not os.access(path, os.X_OK):
        raise BenchmarkConfigError(f"engine path is not executable: {path}")


def run_preflight(engine_path: Path) -> None:
    command = "position startpos\nperft 1\nquit\n"
    try:
        completed = subprocess.run(
            [str(engine_path)],
            input=command,
            text=True,
            capture_output=True,
            check=False,
        )
    except OSError as exc:
        raise BenchmarkConfigError(f"failed to start engine: {exc}") from exc

    if completed.returncode != 0:
        stderr = completed.stderr.strip()
        raise BenchmarkConfigError(f"preflight engine exit code {completed.returncode}: {stderr or 'no stderr'}")
    if completed.stderr.strip():
        raise BenchmarkConfigError(f"preflight wrote to stderr: {completed.stderr.strip()}")

    try:
        output = parse_perft_output(completed.stdout.splitlines(), expected_depth=1)
    except LokiRuntimeError as exc:
        raise BenchmarkConfigError(
            "preflight could not parse perft output; Loki may not be built with LOKI_ENABLE_DEV_COMMANDS"
        ) from exc

    if output.actual_nodes != 20:
        raise BenchmarkConfigError(f"preflight expected 20 startpos nodes at depth 1, got {output.actual_nodes}")


class LokiProcess:
    def __init__(self, engine_path: Path, worker_id: int) -> None:
        self.engine_path = engine_path
        self.worker_id = worker_id
        self.process: asyncio.subprocess.Process | None = None
        self.stderr_queue: asyncio.Queue[str] = asyncio.Queue()
        self.stderr_task: asyncio.Task[None] | None = None

    async def start(self) -> None:
        self.process = await asyncio.create_subprocess_exec(
            str(self.engine_path),
            stdin=asyncio.subprocess.PIPE,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
        )
        self.stderr_queue = asyncio.Queue()
        self.stderr_task = asyncio.create_task(self._drain_stderr())

    async def restart(self) -> None:
        await self.stop()
        await self.start()

    async def stop(self) -> None:
        if self.stderr_task is not None:
            self.stderr_task.cancel()
            try:
                await self.stderr_task
            except asyncio.CancelledError:
                pass
            self.stderr_task = None

        process = self.process
        self.process = None
        if process is None:
            return

        if process.stdin is not None and process.returncode is None:
            try:
                process.stdin.write(b"quit\n")
                await process.stdin.drain()
            except (BrokenPipeError, ConnectionResetError):
                pass

        if process.returncode is None:
            try:
                await asyncio.wait_for(process.wait(), timeout=1.0)
            except asyncio.TimeoutError:
                process.terminate()
                try:
                    await asyncio.wait_for(process.wait(), timeout=1.0)
                except asyncio.TimeoutError:
                    process.kill()
                    await process.wait()

    async def _drain_stderr(self) -> None:
        process = self.process
        if process is None or process.stderr is None:
            return
        while True:
            raw = await process.stderr.readline()
            if not raw:
                break
            await self.stderr_queue.put(raw.decode(errors="replace").rstrip())

    def _require_process(self) -> asyncio.subprocess.Process:
        if self.process is None:
            raise LokiRuntimeError("process_not_started", "worker process has not started")
        if self.process.returncode is not None:
            raise LokiRuntimeError("process_exit", f"engine exited with code {self.process.returncode}")
        if self.process.stdin is None or self.process.stdout is None:
            raise LokiRuntimeError("process_pipe", "engine process is missing stdin/stdout pipes")
        return self.process

    async def run_perft(self, case: PerftCase) -> PerftOutput:
        process = self._require_process()
        while not self.stderr_queue.empty():
            self.stderr_queue.get_nowait()

        command = f"position fen {case.fen}\nperft {case.depth}\n"
        try:
            process.stdin.write(command.encode())
            await process.stdin.drain()
        except (BrokenPipeError, ConnectionResetError) as exc:
            raise LokiRuntimeError("broken_pipe", f"failed to write to engine: {exc}") from exc

        lines = await self._read_perft_lines(case.depth)
        return parse_perft_output(lines, case.depth)

    async def _read_perft_lines(self, depth: int) -> list[str]:
        process = self._require_process()
        assert process.stdout is not None

        started = False
        lines: list[str] = []

        while True:
            stdout_task = asyncio.create_task(process.stdout.readline())
            stderr_task = asyncio.create_task(self.stderr_queue.get())
            done, pending = await asyncio.wait(
                {stdout_task, stderr_task},
                return_when=asyncio.FIRST_COMPLETED,
            )

            for task in pending:
                task.cancel()
            if pending:
                await asyncio.gather(*pending, return_exceptions=True)

            if stderr_task in done:
                message = stderr_task.result()
                stdout_task.cancel()
                await asyncio.gather(stdout_task, return_exceptions=True)
                raise LokiRuntimeError("stderr", message)

            raw = stdout_task.result()
            if not raw:
                returncode = process.returncode
                if returncode is None:
                    returncode = await process.wait()
                raise LokiRuntimeError("process_exit", f"engine exited with code {returncode}")

            line = raw.decode(errors="replace").rstrip()
            if line.startswith("Error:"):
                raise LokiRuntimeError("uci_error", line)

            start_match = START_RE.match(line)
            if start_match:
                if int(start_match.group("depth")) != depth:
                    raise LokiRuntimeError("wrong_perft_depth", f"received start marker for depth {start_match.group('depth')}")
                started = True

            if started:
                lines.append(line)
                end_match = END_RE.match(line)
                if end_match:
                    if int(end_match.group("depth")) != depth:
                        raise LokiRuntimeError("wrong_perft_depth", f"received end marker for depth {end_match.group('depth')}")
                    return lines


def load_textual() -> dict[str, Any]:
    try:
        from textual.app import App, ComposeResult
        from textual.containers import Container, Horizontal, Vertical, VerticalScroll
        from textual.widgets import DataTable, Footer, Header, ProgressBar, RichLog, Static
    except ImportError as exc:
        raise BenchmarkConfigError(
            "Textual is not installed. Run: python -m pip install -r benchmark/requirements.txt"
        ) from exc

    return {
        "App": App,
        "ComposeResult": ComposeResult,
        "Container": Container,
        "Horizontal": Horizontal,
        "Vertical": Vertical,
        "VerticalScroll": VerticalScroll,
        "DataTable": DataTable,
        "Footer": Footer,
        "Header": Header,
        "ProgressBar": ProgressBar,
        "RichLog": RichLog,
        "Static": Static,
    }


def make_benchmark_app(
    textual: dict[str, Any],
    *,
    engine_path: Path,
    perft_jobs: list[PerftJob],
    warnings: list[BenchmarkWarning],
    worker_count: int,
    json_out: Path | None,
):
    App = textual["App"]
    ComposeResult = textual["ComposeResult"]
    Container = textual["Container"]
    Horizontal = textual["Horizontal"]
    Vertical = textual["Vertical"]
    VerticalScroll = textual["VerticalScroll"]
    DataTable = textual["DataTable"]
    Footer = textual["Footer"]
    Header = textual["Header"]
    ProgressBar = textual["ProgressBar"]
    RichLog = textual["RichLog"]
    Static = textual["Static"]

    class PerftBenchmarkApp(App):
        CSS = """
        Screen {
            layout: vertical;
        }

        #run_view {
            height: 1fr;
            layout: vertical;
        }

        #summary {
            height: 4;
            content-align: left middle;
            padding: 0 1;
        }

        #progress {
            height: 3;
            margin: 0 1;
        }

        #tables {
            height: 1fr;
        }

        #workers_panel {
            width: 38%;
            min-width: 42;
            height: 1fr;
        }

        #results_panel {
            width: 62%;
            height: 1fr;
        }

        #worker_table,
        #result_table {
            height: 1fr;
            margin: 0 1 1 1;
        }

        #event_log {
            height: 9;
            margin: 0 1 1 1;
            border: solid $accent;
        }

        #final_view {
            height: 1fr;
            layout: vertical;
        }

        #final_summary {
            height: 5;
            padding: 0 1;
        }

        #error_log {
            height: 1fr;
            margin: 0 1 1 1;
            border: solid $accent;
        }

        .hidden {
            display: none;
        }
        """

        BINDINGS = [
            ("q", "quit", "Quit"),
            ("ctrl+c", "quit", "Cancel/Quit"),
        ]

        def __init__(self) -> None:
            super().__init__()
            self.engine_path = engine_path
            self.perft_jobs = perft_jobs
            self.cases = [case for job in perft_jobs for case in job.cases]
            self.jobs = min(worker_count, len(perft_jobs))
            self.json_out = json_out
            self.queue: asyncio.Queue[PerftJob] = asyncio.Queue()
            self.cancel_event = asyncio.Event()
            self.loki_processes: list[LokiProcess] = []
            self.worker_tasks: list[asyncio.Task[None]] = []
            self.results: list[BenchmarkResult] = []
            self.errors_by_fen: dict[str, list[BenchmarkError]] = defaultdict(list)
            self.warnings_by_fen: dict[str, list[BenchmarkWarning]] = defaultdict(list)
            for warning in warnings:
                self.warnings_by_fen[warning.fen].append(warning)
            self.global_errors: list[str] = []
            self.worker_snapshots = {worker_id: WorkerSnapshot(worker_id=worker_id) for worker_id in range(1, self.jobs + 1)}
            self.summary = BenchmarkSummary(total=len(self.cases), positions=len(perft_jobs), warnings=len(warnings))
            self.started_at = 0.0
            self.exit_code = 2
            self.finished = False

        def compose(self) -> ComposeResult:
            yield Header(show_clock=True)
            with Container(id="run_view"):
                yield Static(id="summary")
                yield ProgressBar(total=len(self.cases), id="progress")
                with Horizontal(id="tables"):
                    with Vertical(id="workers_panel"):
                        yield Static("Workers", classes="panel_title")
                        yield DataTable(id="worker_table")
                    with Vertical(id="results_panel"):
                        yield Static("Results", classes="panel_title")
                        yield DataTable(id="result_table")
                yield RichLog(id="event_log", wrap=True, highlight=True)
            with Container(id="final_view", classes="hidden"):
                yield Static(id="final_summary")
                with VerticalScroll():
                    yield RichLog(id="error_log", wrap=True, highlight=True)
            yield Footer()

        async def on_mount(self) -> None:
            self.title = "Loki Perft Benchmark"
            self._init_tables()
            self._render_summary()
            self.log_event(
                f"Starting {len(self.perft_jobs)} positions / {len(self.cases)} perft runs with {self.jobs} worker(s)."
            )
            self.log_event(f"Engine: {self.engine_path}")
            self.started_at = time.monotonic()
            for job in self.perft_jobs:
                await self.queue.put(job)
            self.worker_tasks = [asyncio.create_task(self._worker_loop(worker_id)) for worker_id in range(1, self.jobs + 1)]
            asyncio.create_task(self._finish_when_done())

        async def action_quit(self) -> None:
            if self.finished:
                self.exit(self.exit_code)
                return

            self.summary.canceled = True
            self.cancel_event.set()
            self.log_event("Cancellation requested. Stopping workers...")
            await self._stop_workers()
            await self._finish()

        def _init_tables(self) -> None:
            worker_table = self.query_one("#worker_table", DataTable)
            worker_table.cursor_type = "row"
            worker_table.add_column("Worker", key="worker")
            worker_table.add_column("State", key="state")
            worker_table.add_column("Position", key="position")
            worker_table.add_column("Depth", key="depth")
            worker_table.add_column("Line", key="line")
            worker_table.add_column("Restarts", key="restarts")
            for worker_id in range(1, self.jobs + 1):
                worker_table.add_row(str(worker_id), "starting", "-", "-", "-", "0", key=f"worker-{worker_id}")

            result_table = self.query_one("#result_table", DataTable)
            result_table.cursor_type = "row"
            result_table.add_column("#", key="ordinal")
            result_table.add_column("Pos", key="position")
            result_table.add_column("Depth", key="depth")
            result_table.add_column("Line", key="line")
            result_table.add_column("Expected", key="expected")
            result_table.add_column("Actual", key="actual")
            result_table.add_column("Status", key="status")
            result_table.add_column("ms", key="elapsed")
            result_table.add_column("NPS", key="nps")
            for case in self.cases:
                result_table.add_row(
                    str(case.ordinal),
                    str(case.position_ordinal),
                    f"D{case.depth}",
                    str(case.line_number),
                    "-" if case.expected_nodes is None else str(case.expected_nodes),
                    "-",
                    "queued",
                    "-",
                    "-",
                    key=f"case-{case.ordinal}",
                )

        def _render_summary(self) -> None:
            wall = max(0.0, time.monotonic() - self.started_at) if self.started_at else 0.0
            self.summary.wall_time_s = wall
            self.summary.aggregate_nps = int(self.summary.total_nodes / wall) if wall > 0 else 0
            text = (
                f"Passed {self.summary.passed} | Failed {self.summary.failed} | "
                f"Warnings {self.summary.warnings} | Runtime errors {self.summary.runtime_errors} | "
                f"Completed {self.summary.completed}/{self.summary.total} | Positions {self.summary.positions} | "
                f"Restarts {self.summary.restarts}\n"
                f"Total nodes {self.summary.total_nodes:,} | Wall {wall:.2f}s | "
                f"Aggregate {self.summary.aggregate_nps:,} nps | Press q to cancel/exit"
            )
            self.query_one("#summary", Static).update(text)

        def _update_worker(self, snapshot: WorkerSnapshot) -> None:
            self.worker_snapshots[snapshot.worker_id] = snapshot
            table = self.query_one("#worker_table", DataTable)
            row_key = f"worker-{snapshot.worker_id}"
            table.update_cell(row_key, "state", snapshot.state)
            table.update_cell(row_key, "position", "-" if snapshot.position_ordinal is None else str(snapshot.position_ordinal))
            table.update_cell(row_key, "depth", "-" if snapshot.depth is None else f"D{snapshot.depth}")
            table.update_cell(row_key, "line", "-" if snapshot.line_number is None else str(snapshot.line_number))
            table.update_cell(row_key, "restarts", str(snapshot.restarts))

        def _record_result(self, result: BenchmarkResult) -> None:
            self.results.append(result)
            self.summary.completed += 1
            if result.status == "passed":
                self.summary.passed += 1
            elif result.status == "failed":
                self.summary.failed += 1
            elif result.status == "error":
                self.summary.runtime_errors += 1
            if result.actual_nodes is not None:
                self.summary.total_nodes += result.actual_nodes

            table = self.query_one("#result_table", DataTable)
            row_key = f"case-{result.ordinal}"
            table.update_cell(row_key, "actual", "-" if result.actual_nodes is None else str(result.actual_nodes))
            table.update_cell(row_key, "status", result.status)
            table.update_cell(row_key, "elapsed", "-" if result.elapsed_ms is None else str(result.elapsed_ms))
            table.update_cell(row_key, "nps", "-" if result.nps is None else str(result.nps))

            progress = self.query_one("#progress", ProgressBar)
            progress.advance(1)
            self._render_summary()

        def _record_error(self, error: BenchmarkError) -> None:
            self.errors_by_fen[error.fen].append(error)

        def _record_global_error(self, message: str) -> None:
            self.global_errors.append(message)
            self.summary.runtime_errors += 1
            self._render_summary()

        def log_event(self, message: str) -> None:
            timestamp = time.strftime("%H:%M:%S")
            self.query_one("#event_log", RichLog).write(f"[{timestamp}] {message}")

        async def _worker_loop(self, worker_id: int) -> None:
            process = LokiProcess(self.engine_path, worker_id)
            self.loki_processes.append(process)
            restarts = 0
            try:
                await process.start()
                self._update_worker(WorkerSnapshot(worker_id=worker_id, state="idle", restarts=restarts))
            except Exception as exc:
                message = f"worker {worker_id}: failed to start: {exc}"
                self.log_event(message)
                self._record_global_error(message)
                self._update_worker(WorkerSnapshot(worker_id=worker_id, state="dead", restarts=restarts))
                return

            while not self.cancel_event.is_set():
                try:
                    job = self.queue.get_nowait()
                except asyncio.QueueEmpty:
                    break

                try:
                    for case in job.cases:
                        if self.cancel_event.is_set():
                            break

                        self._update_worker(
                            WorkerSnapshot(
                                worker_id=worker_id,
                                state="running",
                                position_ordinal=job.ordinal,
                                depth=case.depth,
                                line_number=job.line_number,
                                restarts=restarts,
                            )
                        )

                        try:
                            output = await process.run_perft(case)
                        except LokiRuntimeError as exc:
                            self._record_error(
                                BenchmarkError(
                                    fen=case.fen,
                                    depth=case.depth,
                                    kind=exc.kind,
                                    message=exc.message,
                                    worker_id=worker_id,
                                )
                            )
                            self._record_result(
                                BenchmarkResult(
                                    ordinal=case.ordinal,
                                    position_ordinal=case.position_ordinal,
                                    line_number=case.line_number,
                                    fen=case.fen,
                                    depth=case.depth,
                                    expected_nodes=case.expected_nodes,
                                    status="error",
                                    worker_id=worker_id,
                                    error_kind=exc.kind,
                                    error_message=exc.message,
                                )
                            )
                            self.log_event(f"worker {worker_id}: {exc.kind} on line {case.line_number} D{case.depth}; respawning")
                            restarts += 1
                            self.summary.restarts += 1
                            self._update_worker(
                                WorkerSnapshot(
                                    worker_id=worker_id,
                                    state="respawning",
                                    position_ordinal=job.ordinal,
                                    depth=case.depth,
                                    line_number=job.line_number,
                                    restarts=restarts,
                                )
                            )
                            try:
                                await process.restart()
                            except Exception as restart_exc:
                                message = f"worker {worker_id}: restart failed: {restart_exc}"
                                self.log_event(message)
                                self._record_global_error(message)
                                self.cancel_event.set()
                                break
                            continue
                        except Exception as exc:
                            message = f"unexpected benchmark error: {exc}"
                            self._record_error(
                                BenchmarkError(
                                    fen=case.fen,
                                    depth=case.depth,
                                    kind="unexpected_error",
                                    message=message,
                                    worker_id=worker_id,
                                )
                            )
                            self._record_result(
                                BenchmarkResult(
                                    ordinal=case.ordinal,
                                    position_ordinal=case.position_ordinal,
                                    line_number=case.line_number,
                                    fen=case.fen,
                                    depth=case.depth,
                                    expected_nodes=case.expected_nodes,
                                    status="error",
                                    worker_id=worker_id,
                                    error_kind="unexpected_error",
                                    error_message=message,
                                )
                            )
                            self.log_event(f"worker {worker_id}: unexpected error on line {case.line_number} D{case.depth}; respawning")
                            restarts += 1
                            self.summary.restarts += 1
                            try:
                                await process.restart()
                            except Exception as restart_exc:
                                message = f"worker {worker_id}: restart failed: {restart_exc}"
                                self.log_event(message)
                                self._record_global_error(message)
                                self.cancel_event.set()
                                break
                            continue

                        if case.expected_nodes is None:
                            self._record_result(
                                BenchmarkResult(
                                    ordinal=case.ordinal,
                                    position_ordinal=case.position_ordinal,
                                    line_number=case.line_number,
                                    fen=case.fen,
                                    depth=case.depth,
                                    expected_nodes=None,
                                    actual_nodes=output.actual_nodes,
                                    elapsed_ms=output.elapsed_ms,
                                    nps=output.nps,
                                    status="warning",
                                    worker_id=worker_id,
                                    error_kind="missing_expected_nodes",
                                    error_message="missing expected node count; result was not validated",
                                )
                            )
                        elif output.actual_nodes == case.expected_nodes:
                            self._record_result(
                                BenchmarkResult(
                                    ordinal=case.ordinal,
                                    position_ordinal=case.position_ordinal,
                                    line_number=case.line_number,
                                    fen=case.fen,
                                    depth=case.depth,
                                    expected_nodes=case.expected_nodes,
                                    actual_nodes=output.actual_nodes,
                                    elapsed_ms=output.elapsed_ms,
                                    nps=output.nps,
                                    status="passed",
                                    worker_id=worker_id,
                                )
                            )
                        else:
                            message = f"expected {case.expected_nodes}, got {output.actual_nodes}"
                            self._record_error(
                                BenchmarkError(
                                    fen=case.fen,
                                    depth=case.depth,
                                    kind="incorrect_nodes",
                                    message=message,
                                    worker_id=worker_id,
                                    expected_nodes=case.expected_nodes,
                                    actual_nodes=output.actual_nodes,
                                )
                            )
                            self._record_result(
                                BenchmarkResult(
                                    ordinal=case.ordinal,
                                    position_ordinal=case.position_ordinal,
                                    line_number=case.line_number,
                                    fen=case.fen,
                                    depth=case.depth,
                                    expected_nodes=case.expected_nodes,
                                    actual_nodes=output.actual_nodes,
                                    elapsed_ms=output.elapsed_ms,
                                    nps=output.nps,
                                    status="failed",
                                    worker_id=worker_id,
                                    error_kind="incorrect_nodes",
                                    error_message=message,
                                )
                            )
                            self.log_event(f"line {case.line_number} D{case.depth}: {message}")
                finally:
                    self.queue.task_done()

            await process.stop()
            self._update_worker(WorkerSnapshot(worker_id=worker_id, state="stopped", restarts=restarts))

        async def _finish_when_done(self) -> None:
            await asyncio.gather(*self.worker_tasks, return_exceptions=True)
            await self._finish()

        async def _stop_workers(self) -> None:
            self.cancel_event.set()
            for task in self.worker_tasks:
                if not task.done():
                    task.cancel()
            if self.worker_tasks:
                await asyncio.gather(*self.worker_tasks, return_exceptions=True)
            for process in self.loki_processes:
                await process.stop()

        async def _finish(self) -> None:
            if self.finished:
                return
            self.finished = True
            self.summary.wall_time_s = max(0.0, time.monotonic() - self.started_at)
            self.summary.aggregate_nps = int(self.summary.total_nodes / self.summary.wall_time_s) if self.summary.wall_time_s > 0 else 0
            if self.summary.canceled or self.summary.failed or self.summary.runtime_errors or self.global_errors:
                self.exit_code = 1
            else:
                self.exit_code = 0
            await self._write_json_export()
            self._show_final_summary()

        async def _write_json_export(self) -> None:
            if self.json_out is None:
                return
            payload = {
                "engine": str(self.engine_path),
                "workers": self.jobs,
                "positions": [
                    {
                        "ordinal": job.ordinal,
                        "line_number": job.line_number,
                        "fen": job.fen,
                        "max_depth": job.max_depth,
                        "missing_depths": job.missing_depths,
                    }
                    for job in self.perft_jobs
                ],
                "summary": asdict(self.summary),
                "results": [asdict(result) for result in sorted(self.results, key=lambda item: item.ordinal)],
                "warnings_by_fen": {
                    fen: [asdict(warning) for warning in warnings]
                    for fen, warnings in sorted(self.warnings_by_fen.items(), key=lambda item: item[0])
                },
                "global_errors": self.global_errors,
                "errors_by_fen": {
                    fen: [asdict(error) for error in errors]
                    for fen, errors in sorted(self.errors_by_fen.items(), key=lambda item: item[0])
                },
            }
            try:
                self.json_out.parent.mkdir(parents=True, exist_ok=True)
                self.json_out.write_text(json.dumps(payload, indent=2), encoding="utf-8")
                self.log_event(f"Wrote JSON export to {self.json_out}")
            except OSError as exc:
                self.log_event(f"Failed to write JSON export: {exc}")

        def _show_final_summary(self) -> None:
            self.query_one("#run_view").add_class("hidden")
            self.query_one("#final_view").remove_class("hidden")

            status = "CANCELED" if self.summary.canceled else ("FAILED" if self.exit_code else "PASSED")
            summary = (
                f"{status} | Completed {self.summary.completed}/{self.summary.total} | "
                f"Passed {self.summary.passed} | Failed {self.summary.failed} | "
                f"Warnings {self.summary.warnings} | Runtime errors {self.summary.runtime_errors}\n"
                f"Positions {self.summary.positions} | Worker restarts {self.summary.restarts} | "
                f"Wall {self.summary.wall_time_s:.2f}s | Aggregate {self.summary.aggregate_nps:,} nps\n"
                "Press q to exit."
            )
            self.query_one("#final_summary", Static).update(summary)

            error_log = self.query_one("#error_log", RichLog)
            if not self.global_errors and not self.errors_by_fen and not self.warnings_by_fen:
                error_log.write("No perft mismatches, runtime errors, or warnings were recorded.")
                return

            if self.warnings_by_fen:
                error_log.write("Warnings:")
                for fen, warnings in sorted(self.warnings_by_fen.items(), key=lambda item: item[0]):
                    error_log.write(f"\nFEN: {fen}")
                    for warning in warnings:
                        error_log.write(f"  D{warning.depth} [{warning.kind}]: {warning.message}")

            if self.global_errors:
                error_log.write("\nGlobal errors:")
                for error in self.global_errors:
                    error_log.write(f"  {error}")

            for fen, errors in sorted(self.errors_by_fen.items(), key=lambda item: item[0]):
                error_log.write(f"\nFEN: {fen}")
                for error in errors:
                    worker = "-" if error.worker_id is None else str(error.worker_id)
                    details = f"  D{error.depth} [{error.kind}] worker {worker}: {error.message}"
                    if error.expected_nodes is not None or error.actual_nodes is not None:
                        details += f" (expected={error.expected_nodes}, actual={error.actual_nodes})"
                    error_log.write(details)

    return PerftBenchmarkApp()


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    engine_path = args.engine.resolve()
    epd_path = args.epd.resolve()
    json_out = args.json_out.resolve() if args.json_out is not None else None

    try:
        validate_engine_path(engine_path)
        positions = parse_epd(epd_path)
        perft_jobs, warnings = select_jobs(positions, args.limit)
        run_preflight(engine_path)
        textual = load_textual()
    except BenchmarkConfigError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2

    app = make_benchmark_app(
        textual,
        engine_path=engine_path,
        perft_jobs=perft_jobs,
        warnings=warnings,
        worker_count=args.jobs,
        json_out=json_out,
    )
    result = app.run()
    return int(result if result is not None else 0)


if __name__ == "__main__":
    raise SystemExit(main())
