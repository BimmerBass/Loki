#include <Backend/loki.pch.h>

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
	auto m_ttt = loki::bitboard_t(1) << (static_cast<int64_t>(loki::D5) + 9);
	printBitboard(m_ttt);
	std::cout << "\n\n";
	loki::movegen::magics::slider_generator m_sl;
	printBitboard(m_sl.queen_attacks(loki::D5, m_ttt));
}