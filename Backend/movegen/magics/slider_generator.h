#ifndef SLIDER_GENERATOR_H
#define SLIDER_GENERATOR_H

namespace loki::movegen::magics {

	/// <summary>
	/// Just a wrapper for both the rook and bishop indexes.
	/// </summary>
	class slider_generator {
	private:
		magics_index_t<BISHOP>	m_bishop_index;
		magics_index_t<ROOK>	m_rook_index;

	public:
		slider_generator() {
			m_bishop_index = std::make_unique<magics_index<BISHOP>>();
			m_rook_index = std::make_unique<magics_index<ROOK>>();
		}

		inline bitboard_t rook_attacks(SQUARE sq, bitboard_t occupancy) const noexcept {
			return m_rook_index->attacks_bb(sq, occupancy);
		}
		inline bitboard_t bishop_attacks(SQUARE sq, bitboard_t occupancy) const noexcept {
			return m_bishop_index->attacks_bb(sq, occupancy);
		}
		inline bitboard_t queen_attacks(SQUARE sq, bitboard_t occupancy) const noexcept {
			return m_rook_index->attacks_bb(sq, occupancy) | m_bishop_index->attacks_bb(sq, occupancy);
		}
	};

}

#endif