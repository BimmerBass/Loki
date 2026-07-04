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
