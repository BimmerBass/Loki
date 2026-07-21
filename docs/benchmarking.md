# Benchmarking and Profiling

## Perft benchmark
The Python benchmark runner executes Loki's dev-only `perft` UCI command against the shared [`../tests/perft.epd`](../tests/perft.epd) dataset. Build Loki with dev commands enabled before running the benchmark.

```powershell
cmake --preset release -DLOKI_DEV_COMMANDS=ON
cmake --build --preset release
```

Set up the Python environment from the repository root:

```powershell
python -m venv benchmark\.venv
benchmark\.venv\Scripts\Activate.ps1
python -m pip install -r benchmark\requirements.txt
```

Run the benchmark by pointing it at the Loki executable:

```powershell
python benchmark\perft_benchmark.py --engine .\build\release\bin\Loki.exe --jobs 4
```

Useful options:
- `--jobs N` runs up to `N` Loki worker processes in parallel.
- `--limit N` runs only the first `N` positions.
- `--epd PATH` uses a different EPD dataset instead of `tests/perft.epd`.
- `--json-out PATH` writes full results, warnings, and errors to a JSON file.

For each selected position, the benchmark runs every depth from `1` through the highest depth annotated for that FEN. Missing expected node counts are reported as warnings. Incorrect perft results and runtime errors are grouped by FEN in the final Textual TUI summary.

## Elo gauntlet

`benchmark/gauntlet.py` runs Loki as the seed in a paired-opening fastchess gauntlet and uses Ordo to estimate a local Elo from fixed CCRL opponent anchors. Install fastchess and Ordo separately, then install the existing Python requirements shown above.

Copy [`../benchmark/workloads/initial-elo.example.json`](../benchmark/workloads/initial-elo.example.json), fill in the engines and their ratings, and run:

```powershell
python benchmark\gauntlet.py --workload path\to\initial-elo.json
```

Paths in the workload are relative to the workload file and may contain `${ENV_VAR}` placeholders. Each engine supports `command`, `args`, `working_directory`, and `uci_options`. The match section controls paired games per opponent, concurrency, time control, time margin, opening suite, optional adjudication, and recovery. Score-based draw and resignation adjudication are disabled unless explicitly configured.

Machine-specific placeholder values can be kept in a separate local JSON file:

```json
{
  "FASTCHESS_EXE": "C:/Chess/Tools/fastchess.exe",
  "ORDO_EXE": "C:/Chess/Tools/ordo.exe",
  "LOKI_EXE": "../../build/release/bin/Loki.exe",
  "REFERENCE_ENGINE_EXE": "C:/Chess/Engines/ReferenceEngine.exe",
  "OPENINGS_EPD": "C:/Chess/Books/openings.epd"
}
```

Pass it with `--env-file path\to\gauntlet.local.json`. Values in this file take precedence over actual environment variables; missing names fall back to the process environment. Relative paths in the local file are resolved from the local file's directory. Keep files containing machine-specific paths out of version control.

The Textual interface shows game progress, per-opponent results, warnings, Loki errors, and periodically refreshed Ordo estimates. Press `p` to stop active games and create a durable checkpoint. Continue it with the original workload and the generated state file:

```powershell
python benchmark\gauntlet.py `
  --workload path\to\initial-elo.json `
  --resume-state path\to\gauntlet-state.json
```

Incomplete games are discarded when pausing and replayed after continuation. Completed games remain in `gauntlet.pgn`. Pressing `q` or Ctrl+C cancels instead and removes the resumable manager state.

Every run is written below `benchmark/results/` by default. Its artifacts include the PGN, a game-only `gauntlet.log`, the Loki-only `loki-errors.log`, fastchess and manager state, Ordo text/CSV reports, the resolved workload, and `summary.json`. With `recover: true`, Loki failures are flagged and fastchess continues; otherwise the first Loki failure stops the tournament after recording the failed game.

The displayed value is a **local CCRL-calibrated Elo**, not an official CCRL rating. Hardware, time control, openings, opponent versions, and technical forfeits all affect it.

## Self-play SPRT

`benchmark/sprt.py` tests the Release build of the current working tree against a Release build of a pinned Git revision. The candidate is always listed first. By default the baseline is the current local `HEAD`, so uncommitted engine changes are tested against the commit immediately beneath them:

```powershell
python benchmark\sprt.py `
  --workload benchmark\workloads\sprt-gainer.json `
  --env-file benchmark\workloads\sprt.local.json
```

Pass any locally resolvable Git revision to select another baseline. A committed feature can usually be tested against its parent with `--baseline HEAD^`; a feature branch can be tested against a locally available remote-tracking ref with `--baseline origin/main`.

```powershell
git fetch origin
python benchmark\sprt.py `
  --workload benchmark\workloads\sprt-gainer.json `
  --env-file benchmark\workloads\sprt.local.json `
  --baseline origin/main
```

The runner never fetches, pulls, switches, stashes, resets, or otherwise changes the primary worktree. It configures and builds the candidate in the primary worktree, builds the resolved baseline SHA in a temporary detached worktree, then archives both executables into the result directory before starting Fastchess. If the working tree changes during the candidate build or both archived executables are identical, setup fails rather than running an ambiguous test.

Copy `benchmark/workloads/sprt.local.example.json` to an ignored `*.local.json` file and set the Fastchess and opening-suite paths. Workload paths and `${NAME}` placeholders behave like gauntlet workloads. `sprt.mode` is required:

- `gainer` asks whether the working-tree candidate gains strength and defaults to `<0, 10>`.
- `non_regression` asks whether the working-tree candidate avoids a material loss and defaults to `<-10, 0>`.

Both use normalized Elo, `alpha=0.05`, and `beta=0.05` by default. Any of `elo0`, `elo1`, `alpha`, `beta`, or `model` may be overridden under `sprt`; `model` may be `normalized` or `logistic`. Bayesian SPRT is rejected because Fastchess cannot combine it with the required pentanomial reporting. Normalized Elo accounts for the match's draw rate and is the unit reported by Fastchess when `model` is `normalized`.

Common bounds used by chess-engine testing projects are:

| Engine class / usage | Gainer bounds | Non-regression bounds |
|---|---:|---:|
| Stockfish LTC | `[0.5, 2.5]` | `[-1.75, 0.25]` |
| Stockfish STC | `[0, 2]` | `[-1.75, 0.25]` |
| Top 30 engines | `[0, 3]` | `[-3, 1]` |
| Top 200 engines | `[0, 5]` | `[-5, 0]` |
| All other engines | `[0, 10]` | `[-10, 0]` |

Loki currently uses the last row. Wider bounds reduce the expected game count for a weaker engine, at the cost of requiring a larger gain to pass or tolerating a larger loss in a non-regression test.

Fastchess uses its native pentanomial report (`-report penta=true`). The Textual interface also shows configure/build output, pentanomial results, LLR and its decision thresholds, recent games and their termination reasons, and errors attributed to either engine. Press `p` during the match to stop active games and save a checkpoint, then resume without rebuilding:

```powershell
python benchmark\sprt.py `
  --workload benchmark\workloads\sprt-gainer.json `
  --env-file benchmark\workloads\sprt.local.json `
  --resume-state benchmark\results\loki-feature-gainer\TIMESTAMP\sprt-state.json
```

Resume verifies the workload fingerprint and hashes of both archived executables. Artifacts include the engines, build log and metadata, PGN, game log, per-engine error log, Fastchess state, resolved workload and command, `summary.json`, and a copyable `result.txt` containing the latest complete native Fastchess report, from its separator and `Results of` heading through the pentanomial and LLR lines. Each attributed engine failure in `engine-errors.log` includes its complete matching PGN so the exact game can be reproduced. When Fastchess cannot identify a completed game for an error, the log says so explicitly instead of attaching an unrelated game. When the final Textual screen is closed with `q`, the result block is also printed to the normal terminal for direct use in a commit message.

The exit codes are `0` for H1 accepted, `1` for H0 accepted, `2` for configuration/build errors, `3` for a paused test, `4` for reaching the round cap without a decision, and `5` for cancellation, runtime failure, or a compromised result.

## Profiling with Visual Studio
Build a Release binary with symbols and dev UCI commands enabled:

```powershell
cmake --preset release `
  -DLOKI_DEV_COMMANDS=ON `
  -DCMAKE_CXX_FLAGS_RELEASE="/Zi /O2 /Ob3 /Oi /Ot /GL /fp:fast /arch:AVX2 /GS- /DNDEBUG" `
  -DCMAKE_EXE_LINKER_FLAGS_RELEASE="/DEBUG:FULL /LTCG /OPT:REF /OPT:ICF"

cmake --build --preset release
```

Open Visual Studio, navigate to the Performance Profiler, and run the generated `Loki` executable. The current active benchmark target is perft, since full search is not yet implemented in the 4.0.0 rewrite.
