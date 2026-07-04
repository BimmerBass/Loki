# Implementation
This document describes the active `Loki/` source tree. Older search and evaluation code remains under `Loki.Deprecated/` as reference material, but it is not part of the current build.

## Current engine state
The active 4.0.0 code currently covers:
- UCI command parsing and command registration.
- FEN parsing and FEN serialization.
- Bitboard board representation.
- Magic-bitboard sliding attacks.
- Pseudo-legal move generation for quiet, active, and all moves.
- Legal move filtering through `make_move`.
- Castling, en passant, promotion, move history, and undo handling.
- Perft execution and a Python benchmark runner.

Search and evaluation are not currently implemented in the active tree. Some standard UCI commands are registered for compatibility but currently throw `not_implemented_error`.

## UCI
The engine executable starts a UCI parser loop from `main.cpp`.

Working standard commands include:
- `uci`
- `isready`
- `position`
- `quit`

The `position` command supports both `position startpos` and `position fen ...`, including a following `moves` list.

Registered placeholder commands currently include:
- `go`
- `setoption`
- `debug`
- `stop`
- `ponderhit`
- `register`
- `ucinewgame`

Dev-only commands are compiled when `LOKI_ENABLE_DEV_COMMANDS` is set through the `LOKI_DEV_COMMANDS` CMake option:
- `perft DEPTH`
- `move MOVE`
- `undo`
- `printpos`
- `printmoves [all|quiet|active]`

## Move generation
Loki uses bitboards as its main board representation.

The active move generator includes:
- Hardcoded magic tables for bishop and rook sliding attacks.
- Precomputed knight and king attack tables.
- Template-specialized generation by side and move type.
- Separate quiet, active, and all move generation paths.
- Pawn pushes, captures, promotions, and en passant.
- Castling generation with attacked-square safety checks.
- Attacker lookup helpers for legality checks and king safety.

Legal move validation is handled by making pseudo-legal moves on `search_position` and rejecting moves that leave the moving side's king in check.

## Perft
The active search namespace currently contains perft only. Perft:
- Rebuilds a search position from FEN.
- Generates root moves.
- Recursively makes and undoes legal moves.
- Prints per-root-move node counts, elapsed time, nodes per second, and total nodes.

The benchmark runner in [`../benchmark/perft_benchmark.py`](../benchmark/perft_benchmark.py) drives this command through UCI worker processes and validates results against [`../tests/perft.epd`](../tests/perft.epd).

## Versioning
The build generates version headers from CMake project metadata and git state:
- Release builds report `MAJOR.MINOR.PATCH`.
- Non-release builds append the current commit and branch.
