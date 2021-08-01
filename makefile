


# Operating system
KERNEL = $(shell uname -s)
ifeq ($(KERNEL),Linux)
	OS = $(shell uname -o)
endif

### Definitions
BIT = 64
optimize = yes
use_popcount = yes
perft_transposition_table = no # Only used to make perft faster when testing movegen. Is switched off by default due to size concerns
debug = no


LIBS = -lm -lpthread

### Add compiler sepcific flags
CXXFLAGS = -std=c++17 -lstdc++ -march=native

### Add options
ifeq ($(optimize), yes) # Set optimizations
CXXFLAGS += -O3
endif
ifeq ($(use_popcount), yes) # Use popcount
CXXFLAGS += -DUSE_POPCNT
endif
ifeq ($(BIT), 64) # Compile for 64-bit systems
CXXFLAGS += -DIS_64BIT -m64
else
CXXFLAGS += -m32 # Compile for 32-bit systems
endif
ifeq ($(perft_transposition_table), yes) # Should perft use a transposition table for speed?
CXXFLAGS += -DPERFT_TT
endif
ifeq ($(debug), no) # Set debug mode
CXXFLAGS += -DNDEBUG
endif


SRC_PATH=Loki

FILES=bench.cpp bitboard.cpp evaluation.cpp magics.cpp main.cpp misc.cpp move.cpp \
		movegen.cpp movestager.cpp perft.cpp position.cpp psqt.cpp search.cpp see.cpp \
		thread.cpp transposition.cpp tt_entry.cpp uci.cpp texel.cpp

SOURCES=$(FILES:%.cpp=$(SRC_PATH)/%.cpp)

EXE=Loki3

# Add .exe exstension for windows builds.
ifeq ($(OS), Windows_NT)
EXE = Loki3.exe
endif

all:
	g++ ${SOURCES} ${LIBS} -o $(EXE) ${CXXFLAGS}