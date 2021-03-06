


# Operating system
KERNEL = $(shell uname -s)
ifeq ($(KERNEL),Linux)
	OS = $(shell uname -o)
endif

### Definitions
Bit = 64
optimize = yes
use_popcount = yes
perft_transposition_table = no # Only used to make perft faster when testing movegen. Is switched off by default due to size concerns
debug = no


LIBS = -lm -lpthread

### Add compiler sepcific flags
CXXFLAGS = -std=c++17 -lstdc++ -march=native

### Add options
ifeq ($(optimize), yes)
CXXFLAGS += -O3
endif
ifeq ($(use_popcount), yes)
CXXFLAGS += -DUSE_POPCNT
endif
ifeq ($(Bit), 64)
CXXFLAGS += -DIS_64BIT
endif
ifeq ($(perft_transposition_table), yes)
CXXFLAGS += -DPERFT_TT
endif
ifeq ($(debug), no)
CXXFLAGS += -DNDEBUG
endif


SRC_PATH=Loki

FILES=bench.cpp bitboard.cpp evaluation.cpp magics.cpp main.cpp misc.cpp move.cpp movegen.cpp perft.cpp position.cpp psqt.cpp search.cpp see.cpp thread.cpp transposition.cpp uci.cpp texel.cpp

SOURCES=$(FILES:%.cpp=$(SRC_PATH)/%.cpp)

EXE=Loki3

# Add .exe exstension for windows builds.
ifeq ($(OS), Windows_NT)
EXE = Loki3.exe
endif

all:
	g++ ${SOURCES} ${LIBS} -o $(EXE) ${CXXFLAGS}