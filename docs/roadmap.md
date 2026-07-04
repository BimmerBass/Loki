# Roadmap

## Current rewrite priorities
- Reintroduce a working `go` command.
- Rebuild the search stack on top of the active position and move-generation code.
- Reintroduce evaluation after the current board, UCI, and perft foundation is stable.
- Add UCI options for engine configuration once search exists again.
- Keep expanding tests around move legality, FEN handling, UCI command behavior, and perft coverage.

## Search features to revisit
- Iterative deepening.
- Principal variation search.
- Transposition table.
- Move ordering with hash move, MVV/LVA, history, killer moves, and static exchange evaluation.
- Quiescence search.
- Null move pruning.
- Futility and reverse futility pruning.
- Razoring.
- Late move pruning and reductions.
- Extensions for check and tactical positions.
- Singular extensions.
- Multi-Cut.
- ProbCut.

## Evaluation and tuning
- Rebuild the handcrafted tapered evaluation.
- Revisit material, piece-square tables, pawn structure, passed pawns, mobility, king safety, and piece-specific terms.
- Make the piece evaluation term work well enough to gain Elo.
- Build a tuning workflow for the rebuilt evaluation.
- Build a search tuner based on self-play.
- Consider NNUE or another machine-learning based evaluation once the classical engine is strong and stable again.

## Move generation
- Create a native magic-bitboard implementation instead of relying on copied/reference magic-generation material.
