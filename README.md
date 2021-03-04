# Loki
Loki is a UCI-compliant chess engine written in C++. At the moment it has been estimated to have a strength of ~1773 elo, but this will hopefully be greatly increased.

## Why the name?
After a bit of googling I found someone who recommended that one uses a name that describes what the program does (duh..). Then, I thought that a chess engine is cold, calculating and cunning, so naturally the first name to come to mind
was the nordic god, Loki. After elementary school - where we learned about the nordic gods - I've always thought he was a bit of a d*ck, and so is a chess engine.

#### Special thanks to
- The [Chessprogramming Wiki](https://www.chessprogramming.org/Main_Page) which has been used extensively throughout the creation of Loki.
- The Stockfish source code and community, which has been used where the wiki fell short.
- The creator of [Laser](https://github.com/jeffreyan11/laser-chess-engine) whose implementation of Lazy SMP has served as the inspiration for the one in Loki.
- The [Computer Chess Club](http://www.talkchess.com/forum3/viewforum.php?f=7) which has provided a lot of knowledge and tips.
- The creator of [chess_programming](https://github.com/maksimKorzh/chess_programming) from whom I've taken the magic bitboards implementation.
- [Cute Chess](https://cutechess.com/) the tool used for testing changes and additions.

## Implementation
Loki uses bitboards as its main board representation
#### Move generation
- Magic Bitboards, as implemented by maksimKorzh, for generation of sliding piece attacks.
- Pseudo-legal move generator with legality check in the make move function.
- Overall **Perft @ depth = 5 speed of 450ms** from starting position, without bulk-counting.

#### Evaluation
The evaluation is very simple at the moment, and a good elo increase is expected when I get around to improving it.
- Material.
- Piece-square tables.
- Pawn structure and passed pawns.
- Tapered eval to interpolate between game phases.
- Piece mobility evaluation.
- Specialized piece evaluation. This has been implemented, but showed a small elo decrease, but in the near future, it will be tested in conjunction with mobility in the hopes that they compliment each other.

#### Search
- Lazy SMP supporting up to 8 threads.
- Transposition table supporting sizes from 1MB to 1000MB.
- Iterative deepening.
- Aspiration windows.
- Fail-hard principal variation search.
    - Killer moves.
    - History heuristic.
    - Countermove heuristic.
    - Mvv/Lva for capture sorting.
    - Mate distance pruning.
    - Adaptive Null move pruning with reduction dependant on static_eval - beta which will reduce more aggresively when we're expected to be much above beta, and less when we're not.
    - Enhanced futility pruning.
    - Reverse futility pruning.
    - Razoring.
    - Internal iterative deepening if no hash move has been found.
    - In check extensions and castling extensions.
    - Late move pruning.
    - Late move reductions.
- Quiescence search to resolve captures
    - Delta pruning.
    - Futility pruning for individual moves.
With all the above mentioned move ordering techniques, Loki achieves a cutoff on the first move around 85%-90% of the time.
##### Note: **Late move reductions and late move pruning are disabled at the moment. This is because the evaluation function needs to be more accurate in order for them to work optimally.**

## Building Loki
Loki has been tested to build without errors on both MSVC and GCC (with some warnings by both). If Loki should be compiled to a non-native popcount version one will have to either:
1. If compiling on MSVC, the global preprocessor variable USE_POPCNT should be removed in the project properties.
2. If compiling on GCC, the variable use_popcount in makefile should be set to "no".

It is also possible to change the amount of optimizations with both compilers by (if MSVC) going to the project properties or (if GCC) setting optimize to "no" in the makefile.

##### TO-DO
- Optimize existing pruning techniques.
- Make compilation with GCC more efficient and add native methods for countBits, bitScanForward and bitScanReverse for different systems and architectures.
- Make LMR and LMP more aggresive.
- Try the following additions:
    - Static exhange evaluation.
    - Singular extensions.
    - AEL-pruning.
    - Enhanced forward pruning.
    - Multi-Cut.
    - ProbCut.
    - Fail-High reductions.
    - Null move reductions.
    - Null move threat extensions.
    - King safety in evaluation.
    - Mobility + pieces in evaluation.
- If Loki ever reaches an elo on CCRL of ~2300 I will implement an optimization algorithm called [AdamSPSA](https://arxiv.org/pdf/1910.03591.pdf) 
(see page 4 for parameter updates), in order to tune the evaluation function and search parameters.
- I am very amazed of Stockfish's NNUE evaluation, and if I ever get Loki to play descent chess on CCRL, I will look into creating a new evaluation with some sort of Machine Learning.