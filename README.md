# Loki
Loki is a UCI-compliant chess engine written in C++. At the moment it has been tested to have a strength of 2453 (version 3.0.0 on CCRL blitz) elo.
Loki is not a complete chess program and needs a UCI-compatible GUI (e.g. Arena, Cute Chess, Lucas, Fritz etc..) for comfortable use.

## Why the name?
After a bit of googling I found someone who recommended that one uses a name that describes what the program does (duh..). Then, I thought that a chess engine is cold, calculating and cunning, so naturally the first name to come to mind
was the nordic god, Loki. After elementary school - where we learned about the nordic gods - I've always thought he was a bit of a d*ck, and so is a chess engine.

## Elo history
| Version   | Elo   | TC   |
|-----------|-------|------|
| 1.0.2    | 1766  | 2'+1"|
| 1.2.0     | 1820  | 2'+1"|
| 2.0.0     | 2036  | 2'+1"|
| 3.0.0     | 2453  | 2'+1"|

Loki 3.0.0 is currently being tested with a time control of 40 moves in 15 minutes on [CCRL](https://ccrl.chessdom.com/ccrl/4040/cgi/engine_details.cgi?print=Details&each_game=1&eng=Loki%203.0.0%2064-bit#Loki_3_0_0_64-bit).

## Content
The following files are available in the Loki repository:
- Loki: A directory with all source code included.
- tests: A directory containing python scripts (only one at the moment) used for testing Loki.
- Icon.png and Logo.png: The icon and logo of Loki. These will later be added during compilation.
- Loki.sln: The VS2019 solution for developing Loki.
- README.md: This file.
- makefile: The file used to compile Loki.

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

A tapered eval is used to interpolate between game phases. Additionally, the entire evaluation is tuned using an SPSA-texel optimization algorithm.

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
    - Mate distance pruning.
    - Adaptive Null move pruning.
    - Enhanced futility pruning.
    - Reverse futility pruning.
    - Razoring.
    - ~~Internal iterative deepening if no hash move has been found~~.
    - In check extensions ~~and castling extensions~~.
    - ~~Late move pruning~~.
    - ~~Late move reductions~~.
- Quiescence search to resolve captures
    - ~~Delta pruning~~.
    - ~~Futility pruning for individual moves~~.
    - SEE pruning of bad captures.

With all the above mentioned move ordering techniques, Loki achieves a cutoff on the first move around 85%-90% of the time.
##### Note: **Features with a striketrough (line through the text) are disabled at the moment due to missing elo gains. These will hopefully be successfully implemented in the future.**

## Building Loki
Loki has been tested to build without errors on both MSVC and GCC (with some warnings by the former). If Loki should be compiled to a non-native popcount version one will have to either:
1. If compiling on MSVC, the global preprocessor variable USE_POPCNT should be removed in the project properties.
2. If compiling on GCC, ```use_popcount=no``` should be added when running make.

Additionally, a 32-bit compilation in GCC needs ```Bit=32``` when running make.

It is also possible to change the amount of optimizations with both compilers by (if MSVC) going to the project properties or (if GCC) using ```optimize=no``` when running make.

## Loki's neural network evaluation (LNN)
Loki now features a new kind of evaluation function consisting of a neural network. This has, in contrast to the previous hand-crafted evaluation, been trained using millions of chess positions analyzed by Loki. It was initialized randomly in the beginning, but this does not make the engine Loki0, since the training data generation consisted of previous knowledge.
The difference between the network and HCE (hand-crafted evalution) is that the former has been trained using search scores which more accurately reflects the true value of a position.

The feature has been inspired by the new developments in neural networks on CPUs, as pioneered by the Stockfish team. Note that this in **no way** means that Loki's LNN implementation is a copy of any other engine's or that Loki is a derivative of any other engine. LNN is an original work.

LNN (Loki-neural-network) is disabled by default, but it can be turned on in the engine options.

### Generating training data
Loki features a non-UCI command called "generate". This keyword should be followed by a "type" keyword which should be either "selfplay" or "analysis". The two types of data generation are explained below.
#### Self-play generation
This feature is not supported yet.
#### Analysis generation
This method reads and EPD file (which contains FENs for all positions) and analyzes all positions to a certain depth.

**Mandatory parameters:**
- data: The path to an EPD file containing all training positions.
- output: The path to the output file. This should have a ".lgd" extension, and will be written as a binary file.
- depth: The depth to search each position to.
- threads: The amount of threads to use.

**Optional parameters:**
- limit: This value will exclude all positions with scores outside the interval [-limit, +limit]. By default checkmate scores are excluded.
- batchsize: While the threads analyze positions, they save said positions in LNN's internal input format and the scores. For a big dataset this can exceed the machine's memory. Therefore, the batch size command is used to write a certain number of positions to the output file at a time. The default batch size is 100k.
- hash: The size of the transposition table in megabytes.

#### Example of a data generation command
```
generate type analysis data C:\\Users\\user\\Desktop\\data_epds.epd output C:\\Users\\user\\Desktop\\dataset.lgd depth 4 threads 8 limit 900 batchsize 1000000 hash 256
```

### Training a new network
When training a new network, there are a lot of hyper-parameters that the user needs to determine themselves. Loki features a non-UCI command called "learn" and it has the following:

**Mandatory parameters:**
- dataset: A path to the LGD binary data file.
- epoch: A positive number determining how many times the algorithm should loop through the dataset.
- batchsize: Batch size to divide the training set into.
- loss: The loss function to use. It should be rather easy to implement other loss functions, but Loki currently only has the following two:
    - "mse": Mean squared error.
    - "aae": Average abolute error.
- threads: The amount of threads to use. **Note: Each thread has its own class consisting of all neurons in the network and gradients of all weights and biases. Therefore, a lot of threads are rather memory expensive.**

**Optional parameters:**
- eta: The learning rate to use. Default: 0.01
- eta_decay: The learning rate decay. Default: 0.0001
- batch_load: Due to memory restrictions, big datasets can't be loaded completely. This option determines how many batches to load from the file at a time. Default: 200.
- min_param: If initialized randomly, this determines the minimum parameter values of the weights and biases. Default: -2.0
- max_param: If the network is initialized randomly and min_param has been passes, this should be too. It is the maximum value of each randomly initialized parameter. Default: +2.0
- save_frequency: Amount of epochs before saving the model. A value of 1 saves the model once every epoch. Default: 4
- output: The path to an output file with the ".lnn" extension. Default: LokiNet-{Date and time}.lnn
- net: A path to an already saved model. If this is passed, this network will be loaded and trained further. Default: none.

#### Example of a training session command
```shell
learn dataset C:\\Users\\user\\Desktop\\data.lgd epoch 5 batchsize 1000000 loss aae threads 8 eta 0.001 eta_decay 0.1 batch_load 500 save_frequency 1 output C:\\Users\\user\\Desktop\\new_model.lnn net C:\\Users\\user\\Desktop\\old_model.lnn
```

##### TO-DO
- Try the following additions:
    - Singular extensions.
    - ProbCut.
    - Null move reductions.
    - Null move threat extensions.
- Port the tuning framework to python, and make it work with search tuning.
- Make the evaluation term for pieces work.
- Make LNN more efficient by using quantization and vectorization.
- Create my own magic bitboard implementation. Early in the development of Loki, I didn't want to spend too much time with move generation since my primary goal was to get it to play chess. Therefore, I took the easy way, which is unsatisfactory now... 

### Special thanks to
- The [Chessprogramming Wiki](https://www.chessprogramming.org/Main_Page) which has been used extensively throughout the creation of Loki.
- [BlueFeverSoft](https://github.com/bluefeversoft), the creator of the Vice chess engine. Some of the code in Loki have been inspired from Vice. Additionally, Vice was an excellent resource to use while still getting acquainted with chess programming.
- The Stockfish source code and community, which has been used where the wiki fell short.
- [spsa](https://github.com/zamar/spsa) the repository for tuning StockFish, which has been a big help in implementing Loki's SPSA tuner.
- [Evaluation & Tuning in Chess Engines](https://github.com/AndyGrant/Ethereal/blob/master/Tuning.pdf), a paper written by Andrew Grant (creator of Ethereal), on tuning chess engines, which has contributed to my understanding of the usage of gradient descent algorithms in chess engines.
- The creator of [Laser](https://github.com/jeffreyan11/laser-chess-engine) whose implementation of Lazy SMP has served as the inspiration for the one in Loki.
- The [Computer Chess Club](http://www.talkchess.com/forum3/viewforum.php?f=7) which has provided a lot of knowledge and tips.
- The creator of [chess_programming](https://github.com/maksimKorzh/chess_programming) from whom I've found Tord Romstad's implementation of magic bitboards.
- [Cute Chess](https://cutechess.com/) the tool used for testing changes and additions.
- Marcel Vanthoor, the author of the chess engine [Rustic](https://github.com/mvanthoor/rustic). Between Loki 2.0.0 and Loki 3.0.0, he contacted me saying that Loki was underperforming, its feature set taken into account. If he hadn't told me that, Loki would very likely still be at ~1900-2000 elo.
- Jay Honnold, the creator of the chess engine [Berserk](https://github.com/jhonnold/berserk), who generously let me set up an OpenBench client on his server, which is the tool used for testing changes in Loki currently.
- The creator of [Halogen](https://github.com/KierenP/Halogen) whose neural network implementation was a big help during early LNN development.