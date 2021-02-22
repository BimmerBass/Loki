


# Operating system
KERNEL = $(shell uname -s)
ifeq ($(KERNEL),Linux)
	OS = $(shell uname -o)
endif

### Definitions
Bit = 64
optimize = yes
use_popcount = yes


### Add compiler sepcific flags
CXXFLAGS = -std=c++11 -lstdc++

### Add options
ifeq ($(optimize), yes)
CXXFLAGS += -O3
endif
ifeq ($(use_popcount), yes)
CXXFLAGS += -DUSE_POPCNT
endif
ifeq ($(Bit), 64)
CFLAGS += -DIS_64BIT
endif


SRC_PATH=Loki

FILES=bench.cpp bitboard.cpp evaluation.cpp magics.cpp main.cpp misc.cpp move.cpp movegen.cpp perft.cpp position.cpp psqt.cpp search.cpp thread.cpp transposition.cpp uci.cpp

SOURCES=$(FILES:%.cpp=$(SRC_PATH)/%.cpp)

OUTFILE=Loki.exe

# g++ produces code that is around ten times faster than gcc, but if g++ isn't available, just change it to gcc in the last line
all:
	gcc ${SOURCES} -o $(OUTFILE) ${CXXFLAGS}