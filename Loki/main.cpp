#include "Backend/loki.pch.h"

inline void printBitboard(loki::bitboard_t bb) {
	for (int rank = 7; rank >= 0; rank--) {

		for (int file = 0; file < 8; file++) {
			if (((bb >> (8 * rank + file)) & 1) == 1) {
				std::cout << "X";
			}
			else {
				std::cout << "-";
			}
		}
		std::cout << "\n";
	}
	std::cout << "\n\n";
}

int main() {
	//std::string start_fen = "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2";
	//loki::fen_parser parser;
	//
	//auto pos = parser << start_fen;
	//
	//std::cout << "Parsed FEN!\n";
	//
	//auto generated_fen = parser << pos;
	//
	//std::cout << "Generated: " << generated_fen << std::endl;
	std::string start_fen = "4k3/8/8/8/8/8/8/R3K3 w Q - 0 1";
	loki::position::game_state_t pos = std::make_shared<loki::position::game_state>();
	*pos << start_fen;
	pos->print_position(std::cout);
	std::string tst;
	*pos >> tst;

	std::cout << tst << "\n";
}