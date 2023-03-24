<img src="Logo.png" align="center"/>

## Introduction
Loki is a UCI-compliant chess engine written in C++. At the moment it has been tested to have a strength of 2490 (version 3.5.0 on CCRL 40/15) elo.


## Elo history
| Version   | Elo   | TC   |
|-----------|-------|------|
| 1.0.2    | 1766  | 2'+1"|
| 1.2.0     | 1821  | 2'+1"|
| 2.0.0     | 2036  | 2'+1"|
| 3.0.0     | 2466  | 2'+1"|
| 3.5.0     | 2490  | 40/15|

## Implementation
Loki uses bitboards as its main board representation
#### Move generation
- Magic Bitboards, as implemented by maksimKorzh, for generation of sliding piece attacks.
- Pseudo-legal move generator with legality check in the make move function.
- Overall **Perft @ depth = 5 speed of 290ms** from starting position, without bulk-counting.

#### Evaluation
The evaluation considers the following characteristics of a given position:
- Material.
- Piece-square tables.
- Material imbalances.
- Pawn structure and passed pawns.
- Space term in the middlegame
- Safe piece mobility evaluation.
- King safety evaluation.
- Specialized piece evaluation. This has been implemented, but lost elo, so it is disabled at the moment. I will experiment with it in the future.

A tapered eval is used to interpolate between game phases. Additionally, each thread's evaluation function object has its own evaluation hash table (128KB).

The evaluation function is tuned using an SPSA-texel tuning framework. This will later be changed though.

#### Search
- Lazy SMP supporting up to 8 threads.
- Two-bucket transposition table supporting sizes from 1MB to 1000MB.
- Iterative deepening.
- Aspiration windows.
- Fail-hard principal variation search.
    - Killer moves.
    - History heuristic.
    - ~~Countermove heuristic~~.
    - Mvv/Lva for capture sorting.
    - Static Exchange evaluation for move ordering.
    - Staged move generation.
    - Mate distance pruning.
    - Adaptive Null move pruning.
    - Enhanced futility pruning.
    - Reverse futility pruning.
    - Razoring.
    - ~~Internal iterative deepening if no hash move has been found~~.
    - In check extensions ~~and castling extensions~~.
    - Late move pruning.
    - Late move reductions.
- Quiescence search to resolve captures
    - ~~Delta pruning~~.
    - ~~Futility pruning for individual moves~~.
    - SEE pruning of bad captures.

With all the above mentioned move ordering techniques, Loki achieves a cutoff on the first move around 85%-90% of the time.
##### Note: **Features with a striketrough (line through the text) are disabled at the moment due to missing elo gains. These will hopefully be successfully implemented in the future.**

## Building Loki
Loki has been tested to build without errors on both MSVC and GCC (with some warnings by the former). If Loki should be compiled to a non-native popcount version one will have to either:

- If compiling on MSVC, the global preprocessor variable USE_POPCNT should be removed in the project properties.
- If compiling on GCC, `use_popcount=no` should be added when running make.

Additionally, a 32-bit compilation in GCC needs `BIT=32` when running make. It should be noted however, that 32-bit compilation on 64-bit systems is unstable and should be avoided at the moment.

It is also possible to change the amount of optimizations with both compilers by (if MSVC) going to the project properties or (if GCC) using `optimize=no` when running make.
##### TO-DO
- Try the following additions:
    - Singular extensions.
    - Multi-Cut.
    - ProbCut.
    - Null move reductions.
    - Null move threat extensions.
- Make a real gradient-based (in contrast to SPSA that only has gradient directions) evaluation tuner.
- Make a search-tuner that uses self-play.
- Make the evaluation term for pieces work.
- I am very amazed of Stockfish's NNUE evaluation, and if I ever get Loki to play descent chess on CCRL, I will look into creating a new evaluation with some sort of Machine Learning.
- Create my own magic bitboard implementation. Early in the development of Loki, I didn't want to spend too much time with move generation since my primary goal was to get it to play chess. Therefore, I took the easy way, which is unsatisfactory now... 

#### Special thanks to
- The [Chessprogramming Wiki](https://www.chessprogramming.org/Main_Page) which has been used extensively throughout the creation of Loki.
- [BlueFeverSoft](https://github.com/bluefeversoft), the creator of the Vice chess engine. Some of the code in Loki have been inspired from Vice. This is especially true for the UCI-implementation, which has nearly been copied.
- The Stockfish source code and community, which has been used where the wiki fell short.
- [spsa](https://github.com/zamar/spsa) the repository for tuning StockFish, which has been a big help in implementing Loki's SPSA tuner.
- [Evaluation & Tuning in Chess Engines](https://github.com/AndyGrant/Ethereal/blob/master/Tuning.pdf), a paper written by Andrew Grant (creator of Ethereal), on tuning chess engines, which has contributed to my understanding of the usage of gradient descent algorithms in chess engines.
- The creator of [Laser](https://github.com/jeffreyan11/laser-chess-engine) whose implementation of Lazy SMP has served as the inspiration for the one in Loki.
- The [Computer Chess Club](http://www.talkchess.com/forum3/viewforum.php?f=7) which has provided a lot of knowledge and tips.
- The creator of [chess_programming](https://github.com/maksimKorzh/chess_programming) from whom I've found Tord Romstad's implementation of magic bitboards.
- [Cute Chess](https://cutechess.com/) the tool used for testing changes and additions.
- Marcel Vanthoor, the author of the chess engine [Rustic](https://github.com/mvanthoor/rustic). Between Loki 2.0.0 and Loki 3.0.0, he contacted me saying that Loki was underperforming, its feature set taken into account. If he hadn't told me that, Loki would very likely still be at ~1900-2000 elo.
- Jay Honnold, the creator of the chess engine [Berserk](https://github.com/jhonnold/berserk), who generously let me set up an OpenBench client on his server, which is the tool used for testing changes in Loki currently.