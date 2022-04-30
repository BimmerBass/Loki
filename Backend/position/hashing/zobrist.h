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
#ifndef ZOBRIST_H
#define ZOBRIST_H

namespace loki::position {

	class zobrist {
		friend class ::loki::utility::initializer;
	private:
		static std::array<
			std::array<
			std::unique_ptr<hashkey_t[]>, // 64 squares.
			PIECE_NB>,
			SIDE_NB>							piece_hashes;
		static std::unique_ptr<hashkey_t[]>	ep_hashes;
		static std::unique_ptr<hashkey_t[]>	castling_hashes;
		static hashkey_t						stm_hash;

	public:
		inline void toggle_piece(hashkey_t& key, SIDE s, PIECE pce, size_t sq) const noexcept {
			key ^= piece_hashes[s][pce][sq];
		}
		inline void toggle_ep(hashkey_t& key, size_t ep_sq) const noexcept {
			key ^= ep_hashes[ep_sq];
		}
		inline void toggle_castling(hashkey_t& key, uint8_t cr) {
			key ^= castling_hashes[cr];
		}
		inline void toggle_stm(hashkey_t& key) {
			key ^= stm_hash;
		}

	private:
		static void init();
	};

}

#endif