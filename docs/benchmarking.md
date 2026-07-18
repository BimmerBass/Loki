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
