


# Operating system
KERNEL = $(shell uname -s)
ifeq ($(KERNEL),Linux)
	OS = $(shell uname -o)
endif

### Definitions
Bit = 32
optimize = yes
use_popcount = yes


### Add compiler sepcific flags
CFLAGS = -std=gnu++11 -pthread

ifeq ($(optimize), yes)
CFLAGS += -O3
endif

##ifeq ($(use_popcount), yes)
##CFLAGS += -DUSE_POPCNT
##endif
##
##ifeq ($(Bit), 64)
##CFLAGS += -DIS_64BIT
##endif





SRC_PATH=Loki

FILES=bench.cpp bitboard.cpp evaluation.cpp magics.cpp main.cpp misc.cpp move.cpp movegen.cpp perft.cpp position.cpp psqt.cpp search.cpp thread.cpp transposition.cpp uci.cpp

SOURCES=$(FILES:%.cpp=$(SRC_PATH)/%.cpp)

OUTFILE=Loki.exe

all:
	g++ ${SOURCES} -o $(OUTFILE) ${CFLAGS}