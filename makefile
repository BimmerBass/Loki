SRC_PATH=Loki

FILES=bench.cpp bitboard.cpp evaluation.cpp magics.cpp main.cpp misc.cpp move.cpp movegen.cpp perft.cpp position.cpp psqt.cpp search.cpp thread.cpp transposition.cpp uci.cpp

SOURCES=$(FILES:%.cpp=$(SRC_PATH)/%.cpp)
CLFAGS=-std=c++14 -O3

VERSION=1.0.2
OUTFILE=Loki $(VERSION)

all:
	g++ ${SOURCES} -o OUTFILE &{CFLAGS}