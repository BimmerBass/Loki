#include "Backend/loki.pch.h"

int main() {
	std::string start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	std::cout << "Starting perft\n";

	using namespace loki;
	auto m_pos = position::position::create_position(
		std::make_shared<position::game_state>(),
		std::make_shared<movegen::magics::slider_generator>());
	*m_pos << start_fen;
	std::cout << m_pos << std::endl;

	//loki::utility::perft pft(start_fen);
	//
	//pft.perform(loki::DEPTH(6), std::cout, true);
}