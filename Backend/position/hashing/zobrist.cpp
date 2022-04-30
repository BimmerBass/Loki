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
#include "loki.pch.h"


namespace loki::position {

	std::array<
		std::array<
		std::unique_ptr<hashkey_t[]>, // 64 squares.
		PIECE_NB>,
		SIDE_NB>					zobrist::piece_hashes{};
	std::unique_ptr<hashkey_t[]>	zobrist::ep_hashes{};
	std::unique_ptr<hashkey_t[]>	zobrist::castling_hashes{};
	hashkey_t						zobrist::stm_hash{};

	/// <summary>
	/// Initialize the tables of random numbers used by the hashing algorithm.
	/// </summary>
	void zobrist::init() {
		std::mt19937_64 rng(0x1234);

		ep_hashes		= std::make_unique<hashkey_t[]>(SQ_NB);
		castling_hashes = std::make_unique<hashkey_t[]>(16);
		stm_hash = rng();

		for (size_t pce = PAWN; pce <= KING; pce++) {
			piece_hashes[WHITE][pce] = std::make_unique<hashkey_t[]>(SQ_NB);
			piece_hashes[BLACK][pce] = std::make_unique<hashkey_t[]>(SQ_NB);

			for (size_t sq = A1; sq <= H8; sq++) {
				piece_hashes[WHITE][pce][sq] = rng();
				piece_hashes[BLACK][pce][sq] = rng();
			}
		}
		for (size_t sq = A1; sq <= H8; sq++) {
			ep_hashes[sq] = rng();
		}
		for (size_t i = 0; i < 16; i++) {
			castling_hashes[i] = rng();
		}
	}

}