// Loki, a UCI-compliant chess playing software
// Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
// Loki is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Loki is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "pch.hpp"
#include "Loki/movegen/magics/generation/rook_generator.hpp"

namespace movegen_tests::generation_tests::magics_tests
{
	using namespace loki::position;
	using namespace loki::movegen::magics::generation;

	static const std::array<std::tuple<square, bitboard_t, bitboard_t>, 6> rook_cases{{
		{A1, 0x01010101010101feULL, 0x0ULL},
		{H5, 0x8080807f80808080ULL, 0x0ULL},
		{E4, 0x10101010ef101010ULL, 0x0ULL},
		{E5, 0x101010ec10101010ULL, 0x0000000400000000ULL},
		{C4, 0x000000041b040404ULL, 0x0000000411000000ULL},
		{F6, 0x2020df2020202020ULL, 0x0000000200040000ULL},
	}};

	TEST_CASE("rook attacks match expected values", "[movegen][magics][rook]")
	{
		rook_generator gen;
		for (const auto& [sq, expected, occ] : rook_cases)
		{
			SECTION(sq.to_algebraic())
			{
				REQUIRE(gen.attack(sq, occ) == expected);
			}
		}
	}

	TEST_CASE("rook occupancy masks exclude the board edge", "[movegen][magics][rook]")
	{
		rook_generator gen;
		for (square sq = A1; sq <= H8; ++sq)
		{
			SECTION(sq.to_algebraic())
			{
				bitboard_t border = 0ULL;
				if (sq.rank() != RANK_1)
					border |= RANK_MASKS[RANK_1];
				if (sq.rank() != RANK_8)
					border |= RANK_MASKS[RANK_8];
				if (sq.file() != FILE_A)
					border |= FILE_MASKS[FILE_A];
				if (sq.file() != FILE_H)
					border |= FILE_MASKS[FILE_H];

				const auto mask = gen.occupancy_mask(sq);
				const auto attack = gen.attack(sq, 0ULL);
				REQUIRE(mask == (attack & ~border));
			}
		}
	}
}
