#include "Backend/loki.pch.h"

int main() {
	std::string start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	std::cout << "Starting perft\n";

	loki::utility::perft pft(start_fen);

	pft.perform(loki::DEPTH(6), std::cout, true);
}