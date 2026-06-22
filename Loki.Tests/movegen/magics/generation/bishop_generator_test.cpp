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
#include "Loki/movegen/magics/generation/bishop_generator.hpp"

namespace movegen_tests::generation_tests::magics_tests
{
	using namespace loki::position;
	using namespace loki::movegen::magics::generation;

	static const std::array<std::tuple<square, bitboard_t, bitboard_t>, 6> bishop_cases{{
		{A1, 0x8040201008040200ULL, 0x0ULL},
		{H5, 0x1020400040201008ULL, 0x0ULL},
		{E5, 0x8244280028448201ULL, 0x0ULL},
		{E4, 0x0182442800284080ULL, 0x0000000000080000ULL},
		{C4, 0x0000010a000a1100ULL, 0x0000010804001000ULL},
		{F6, 0x8850005088040201ULL, 0x0080220000000000ULL},
	}};

	TEST_CASE("bishop attacks match expected values", "[movegen][magics][bishop]")
	{
		bishop_generator gen;
		for (const auto& [sq, expected, occ] : bishop_cases)
		{
			SECTION(sq.to_algebraic())
			{
				REQUIRE(gen.attack(sq, occ) == expected);
			}
		}
	}

	TEST_CASE("bishop occupancy masks exclude the board edge", "[movegen][magics][bishop]")
	{
		bishop_generator gen;
		const auto border_mask = FILE_MASKS[FILE_A] | FILE_MASKS[FILE_H] | RANK_MASKS[RANK_1] | RANK_MASKS[RANK_8];
		for (square sq = A1; sq <= H8; ++sq)
		{
			SECTION(sq.to_algebraic())
			{
				const auto mask = gen.occupancy_mask(sq);
				const auto attack = gen.attack(sq, 0ULL);
				REQUIRE(mask == (attack & ~border_mask));
			}
		}
	}
}
