//
//	Loki, a UCI-compliant chess playing software
//	Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
//	Loki is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	Loki is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
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