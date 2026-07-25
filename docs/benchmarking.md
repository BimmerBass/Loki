# Benchmarking and profiling

## Setup

Create the shared Python environment from the repository root:

```powershell
python -m venv benchmark\.venv
benchmark\.venv\Scripts\Activate.ps1
python -m pip install -r benchmark\requirements.txt
```

## Perft

Build Loki with development commands enabled:

```powershell
cmake --preset release -DLOKI_DEV_COMMANDS=ON
cmake --build --preset release --target Loki
```

Run the [`perft.epd`](../tests/perft.epd) suite:

```powershell
python benchmark\perft_benchmark.py `
  --engine .\build\release\bin\Loki.exe `
  --jobs 4
```

Add `--validate-hashing` to recompute and verify the Zobrist key at every leaf:

```powershell
python benchmark\perft_benchmark.py `
  --engine .\build\release\bin\Loki.exe `
  --jobs 4 `
  --validate-hashing
```

Hash validation runs to the full requested depth and is substantially slower than normal perft. The equivalent direct development command is:

```text
perft <depth> hashvalidate
```

Other useful options are `--limit N`, `--epd PATH`, and `--json-out PATH`. The runner tests every depth through the highest one specified for each FEN, reports missing expectations as warnings, and groups failures by FEN.

## Elo gauntlet

[`gauntlet.py`](../benchmark/gauntlet.py) runs paired-opening Fastchess matches and uses Ordo to estimate a local Elo from fixed CCRL anchors. Install Fastchess and Ordo separately.

Copy [`initial-elo.example.json`](../benchmark/workloads/initial-elo.example.json), configure the engines and ratings, then run:

```powershell
python benchmark\gauntlet.py `
  --workload path\to\initial-elo.json `
  --env-file path\to\gauntlet.local.json
```

Workload paths are relative to the workload file and may use `${ENV_VAR}` placeholders. An optional local environment JSON file supplies machine-specific paths and takes precedence over process environment variables.

Press `p` to stop active games and save a checkpoint. Resume with:

```powershell
python benchmark\gauntlet.py `
  --workload path\to\initial-elo.json `
  --resume-state path\to\gauntlet-state.json
```

Results are written below `benchmark/results/` and include PGN, logs, state, Ordo reports, the resolved workload, and `summary.json`. The reported Elo is local to the configured hardware, time control, openings, opponents, and engine versions; it is not an official CCRL rating.

## Self-play SPRT

[`sprt.py`](../benchmark/sprt.py) compares the Release build of the working tree with a Release build of a local Git revision. The default baseline is `HEAD`, which is useful for testing uncommitted changes:

```powershell
python benchmark\sprt.py `
  --workload benchmark\workloads\sprt-gainer.json `
  --env-file benchmark\workloads\sprt.local.json
```

Select another local revision with `--baseline`, for example `HEAD^` or `origin/main`. The runner does not fetch or modify the primary worktree; it builds the baseline in a temporary detached worktree and archives both executables before the match.

The workload must select one mode:

- `gainer` tests for an improvement and defaults to `<0, 10>`.
- `non_regression` tests against a material loss and defaults to `<-10, 0>`.

Default bounds by engine class are:

| Engine class | Gainer | Non-regression |
|---|---:|---:|
| Stockfish LTC | `[0.5, 2.5]` | `[-1.75, 0.25]` |
| Stockfish STC | `[0, 2]` | `[-1.75, 0.25]` |
| Top 30 | `[0, 3]` | `[-3, 1]` |
| Top 200 | `[0, 5]` | `[-5, 0]` |
| Other, including Loki | `[0, 10]` | `[-10, 0]` |

The defaults use normalized Elo with `alpha=0.05`, `beta=0.05`, and pentanomial reporting. Override `elo0`, `elo1`, `alpha`, `beta`, or `model` under `sprt`.

Press `p` to save a checkpoint, then resume without rebuilding:

```powershell
python benchmark\sprt.py `
  --workload benchmark\workloads\sprt-gainer.json `
  --env-file benchmark\workloads\sprt.local.json `
  --resume-state path\to\sprt-state.json
```

Artifacts include both engines, build metadata, PGN, logs, Fastchess state, the resolved workload, `summary.json`, and `result.txt`. Exit codes are `0` for H1, `1` for H0, `2` for setup errors, `3` for paused, `4` for no decision at the round cap, and `5` for cancellation or runtime failure.

## Visual Studio profiling

Build an optimized binary with symbols and development commands:

```powershell
cmake --preset release `
  -DLOKI_DEV_COMMANDS=ON `
  -DCMAKE_CXX_FLAGS_RELEASE="/Zi /O2 /Ob3 /Oi /Ot /GL /fp:fast /arch:AVX2 /GS- /DNDEBUG" `
  -DCMAKE_EXE_LINKER_FLAGS_RELEASE="/DEBUG:FULL /LTCG /OPT:REF /OPT:ICF"

cmake --build --preset release
```

Open the generated `Loki` executable with Visual Studio's Performance Profiler.
