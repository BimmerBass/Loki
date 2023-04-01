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
#include "loki.pch.hpp"

#undef S
#define S(v) static_cast<eValue>( (v) )

namespace loki::evaluation
{

	void hardcoded_params::initialize()
	{
		// Rustic's eval parameters.
		pawn = static_cast<eValue>(512);
		knight = static_cast<eValue>(1587);
		bishop = static_cast<eValue>(1638);
		rook = static_cast<eValue>(2560);
		queen = static_cast<eValue>(4608);

		int rustic_psqt[6][64] =
		{
			{ // PAWN
				 0,   0,   0,   0,   0,   0,   0,   0,
				60,  60,  60,  60,  70,  60,  60,  60,
				40,  40,  40,  50,  60,  40,  40,  40,
				20,  20,  20,  40,  50,  20,  20,  20,
				 5,   5,  15,  30,  40,  10,   5,   5,
				 5,   5,  10,  20,  30,   5,   5,   5,
				 5,   5,   5, -30, -30,   5,   5,   5,
				 0,   0,   0,   0,   0,   0,   0,   0
			},
			{ // KNIGHT
				-20, -10,  -10,  -10,  -10,  -10,  -10,  -20,
				-10,  -5,   -5,   -5,   -5,   -5,   -5,  -10,
				-10,  -5,   15,   15,   15,   15,   -5,  -10,
				-10,  -5,   15,   15,   15,   15,   -5,  -10,
				-10,  -5,   15,   15,   15,   15,   -5,  -10,
				-10,  -5,   10,   15,   15,   15,   -5,  -10,
				-10,  -5,   -5,   -5,   -5,   -5,   -5,  -10,
				-20,   0,  -10,  -10,  -10,  -10,    0,  -20
			},
			{ // BISHOP
				-20,    0,    0,    0,    0,    0,    0,  -20,
				-15,    0,    0,    0,    0,    0,    0,  -15,
				-10,    0,    0,    5,    5,    0,    0,  -10,
				-10,   10,   10,   30,   30,   10,   10,  -10,
				  5,    5,   10,   25,   25,   10,    5,    5,
				  5,    5,    5,   10,   10,    5,    5,    5,
				-10,    5,    5,   10,   10,    5,    5,  -10,
				-20,  -10,  -10,  -10,  -10,  -10,  -10,  -20
			},
			{ // ROOK
				0,   0,   0,   0,   0,   0,   0,   0,
				15,  15,  15,  20,  20,  15,  15,  15,
				0,   0,   0,   0,   0,   0,   0,   0,
				0,   0,   0,   0,   0,   0,   0,   0,
				0,   0,   0,   0,   0,   0,   0,   0,
				0,   0,   0,   0,   0,   0,   0,   0,
				0,   0,   0,   0,   0,   0,   0,   0,
				0,   0,   0,  10,  10,  10,   0,   0
			},
			{ // QUEEN
				-30,  -20,  -10,  -10,  -10,  -10,  -20,  -30,
				-20,  -10,   -5,   -5,   -5,   -5,  -10,  -20,
				-10,   -5,   10,   10,   10,   10,   -5,  -10,
				-10,   -5,   10,   20,   20,   10,   -5,  -10,
				-10,   -5,   10,   20,   20,   10,   -5,  -10,
				-10,   -5,   -5,   -5,   -5,   -5,   -5,  -10,
				-20,  -10,   -5,   -5,   -5,   -5,  -10,  -20,
				-30,  -20,  -10,  -10,  -10,  -10,  -20,  -30
			},
			{ // KING
			0,    0,     0,     0,    0,    0,    0,    0,
			0,    0,     0,     0,    0,    0,    0,    0,
			0,    0,     0,     0,    0,    0,    0,    0,
			0,    0,     0,    20,   20,    0,    0,    0,
			0,    0,     0,    20,   20,    0,    0,    0,
			0,    0,     0,     0,    0,    0,    0,    0,
			0,    0,     0,   -10,  -10,    0,    0,    0,
			0,    0,    20,   -10,  -10,    0,   20,    0,
			}
		};
		std::array<std::array<eValue, SQ_NB>, PIECE_NB> loki_psqt;

		for (auto i = PAWN; i <= KING; i++)
		{
			for (auto sq = A1; sq <= H8; sq++)
			{
				auto rustic_val = rustic_psqt[i][make_side_relative<BLACK>(sq)];
				loki_psqt[i][sq] = static_cast<eValue>(std::round(double(rustic_val) * 5.12));
			}
		}
		piece_square_tables = PieceSquareTables(
			std::initializer_list<eValue>(loki_psqt[PAWN].data(), loki_psqt[PAWN].data() + (size_t)SQ_NB),
			std::initializer_list<eValue>(loki_psqt[KNIGHT].data(), loki_psqt[KNIGHT].data() + (size_t)SQ_NB),
			std::initializer_list<eValue>(loki_psqt[BISHOP].data(), loki_psqt[BISHOP].data() + (size_t)SQ_NB),
			std::initializer_list<eValue>(loki_psqt[ROOK].data(), loki_psqt[ROOK].data() + (size_t)SQ_NB),
			std::initializer_list<eValue>(loki_psqt[QUEEN].data(), loki_psqt[QUEEN].data() + (size_t)SQ_NB),
			std::initializer_list<eValue>(loki_psqt[KING].data(), loki_psqt[KING].data() + (size_t)SQ_NB));

		tempo = static_cast<eValue>(26);
	}

}

#undef S