<p align="center">
  <img src="Logo.png" style="width: 30%" />
</p>

---

Loki is a UCI-compliant chess engine written in C++23. The current source tree is the 4.0.0 rewrite, focused on the modern CMake build, UCI plumbing, position representation, legal move generation, and perft validation.

The last tested pre-rewrite playing release listed here is version 3.5.0, which reached 2490 Elo on CCRL 40/15. The current 4.0.0 code does not yet have the full search and evaluation stack wired back in.

## Requirements
- CMake 3.24 or newer.
- A C++23 compliant compiler.
- Visual Studio 18 2026 for the checked-in CMake presets.

## Quick start
Configure, build, and test the Debug preset:

```powershell
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Build the optimized Release preset:

```powershell
cmake --preset release
cmake --build --preset release
ctest --preset release
```

For more detail, see:
- [Building Loki](docs/building.md)
- [Benchmarking and profiling](docs/benchmarking.md)

## Current status
The active implementation currently includes:
- UCI command parsing and command registration.
- Working `uci`, `isready`, `position`, `ucinewgame`, and `quit` commands.
- FEN parsing and `position startpos` / `position fen ... moves ...` support.
- Bitboard-based board state.
- Magic-bitboard sliding attacks.
- Legal move generation through pseudo-legal generation plus make-move legality checks.
- Move history and undo support.
- Dev-only commands for perft, move inspection, manual moves, undo, and position printing.
- Catch2-based tests and a Python Textual perft benchmark runner.

Search and evaluation are not currently implemented in the active `Loki/` tree. Some UCI commands, including `go`, `setoption`, `debug`, `stop`, `ponderhit`, and `register`, are registered placeholders that currently throw `not_implemented_error`.

For deeper technical notes, see [Implementation](docs/implementation.md).

## Elo history
| Version | Elo | TC |
|---------|-----|----|
| 1.0.2 | 1766 | 2'+1" |
| 1.2.0 | 1821 | 2'+1" |
| 2.0.0 | 2036 | 2'+1" |
| 3.0.0 | 2466 | 2'+1" |
| 3.5.0 | 2490 | 40/15 |

## Roadmap
Current rewrite tasks and future engine work are tracked in [Roadmap](docs/roadmap.md).

## Special thanks
- The [Chessprogramming Wiki](https://www.chessprogramming.org/Main_Page), an essential reference throughout Loki's development.
- [BlueFeverSoft](https://github.com/bluefeversoft), whose Vice chess engine and tutorial series got me into chess programming.
- The Stockfish source code and community, for practical examples and ideas where the wiki was not enough.
- [Evaluation & Tuning in Chess Engines](https://github.com/AndyGrant/Ethereal/blob/master/Tuning.pdf), Andrew Grant's paper on tuning chess engines, for improving my understanding of engine tuning.
- The [Computer Chess Club](http://www.talkchess.com/forum3/viewforum.php?f=7), for years of shared computer-chess knowledge and discussion.
- The creator of [chess_programming](https://github.com/maksimKorzh/chess_programming), where I found Tord Romstad's implementation of magic bitboards.
- [Cute Chess](https://cutechess.com/), the tool used for testing changes and additions.
- Marcel Vanthoor, the author of [Rustic](https://github.com/mvanthoor/rustic), for pointing out after Loki 2.0.0 that the engine was underperforming relative to its feature set. That feedback likely saved Loki from staying around 1900-2000 Elo.
- Jay Honnold, the creator of [Berserk](https://github.com/jhonnold/berserk), for generously supporting Loki's testing work in the past.
