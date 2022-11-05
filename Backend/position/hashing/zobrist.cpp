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


namespace loki::position
{

	/// <summary>
	/// Initialize the tables of random numbers used by the hashing algorithm.
	/// Note: even though all positions should hash the same way, we can achieve this by using a fixed seed for the PRNG.
	/// </summary>
	zobrist::zobrist()
	{
		std::mt19937_64 rng(0x1234);

		m_ep_hashes = std::make_unique<hashkey_t[]>(SQ_NB);
		m_castling_hashes = std::make_unique<hashkey_t[]>(16);
		m_stm_hash = rng();

		for (size_t pce = PAWN; pce <= KING; pce++)
		{
			m_piece_hashes[WHITE][pce] = std::make_unique<hashkey_t[]>(SQ_NB);
			m_piece_hashes[BLACK][pce] = std::make_unique<hashkey_t[]>(SQ_NB);

			for (size_t sq = A1; sq <= H8; sq++)
			{
				m_piece_hashes[WHITE][pce][sq] = rng();
				m_piece_hashes[BLACK][pce][sq] = rng();
			}
		}
		for (size_t sq = A1; sq <= H8; sq++)
		{
			m_ep_hashes[sq] = rng();
		}
		for (size_t i = 0; i < 16; i++)
		{
			m_castling_hashes[i] = rng();
		}
	}

}