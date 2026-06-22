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
#include "Loki/position/bitboard.hpp"

namespace position_tests
{
	using namespace loki::position;

	TEST_CASE("bitboard helpers detect and set bits", "[position][bitboard]")
	{
		REQUIRE_FALSE(is_one_at(0x1ULL, 1));
		REQUIRE(is_one_at(0x1ULL, 0));

		bitboard_t zero = 0;
		REQUIRE(pop_lsb(zero) == std::nullopt);
		REQUIRE(zero == 0);

		bitboard_t nonzero = (bitboard_t(1) << 32) | (bitboard_t(1) << 63);
		REQUIRE(pop_lsb(nonzero) == std::optional<size_t>{32});
		REQUIRE(nonzero == (1ULL << 63));
	}

	TEST_CASE("bitboard popcount matches reference values", "[position][bitboard]")
	{
		const std::array<std::pair<bitboard_t, size_t>, 3> cases{{
			{0ULL, 0},
			{~0ULL, 64},
			{0x5555555555555555ULL, 32},
		}};

		for (const auto& [value, expected] : cases)
		{
			SECTION(std::to_string(expected))
			{
				REQUIRE(popcount(value) == expected);
			}
		}
	}

	TEST_CASE("bitboard scans find least and most significant bits", "[position][bitboard]")
	{
		const std::array<std::tuple<bitboard_t, std::optional<size_t>, std::optional<size_t>>, 3> cases{{
			{0ULL, std::nullopt, std::nullopt},
			{0x000f000ULL, 12ULL, 15ULL},
			{0x8000000000000001ULL, 0ULL, 63ULL},
		}};

		for (const auto& [value, lsb, msb] : cases)
		{
			SECTION(std::to_string(value))
			{
				REQUIRE(scan_lsb(value) == lsb);
				REQUIRE(scan_msb(value) == msb);
			}
		}
	}

	TEST_CASE("bitboard set and toggle helpers behave as expected", "[position][bitboard]")
	{
		for (size_t index : {0ULL, 63ULL, 32ULL})
		{
			SECTION(std::to_string(index))
			{
				REQUIRE(set_one_at(0, index) == (bitboard_t(1) << index));
				const auto initial = bitboard_t(1) << index;
				REQUIRE(set_one_at(initial, index) == initial);
				REQUIRE(toggle_at(initial, index) == 0ULL);
				REQUIRE(toggle_at(0ULL, index) == (bitboard_t(1) << index));
			}
		}
	}

	TEST_CASE("bitboard shifts respect board edges", "[position][bitboard]")
	{
		const std::array<std::tuple<bitboard_t, bitboard_t, e_direction>, 12> cases{{
			{0xFF00000000000000ULL, 0x0000000000000000ULL, UP},
			{0xFF00000000000000ULL, 0x00FF000000000000ULL, DOWN},
			{0x00000000000000FFULL, 0x000000000000FF00ULL, UP},
			{0x00000000000000FFULL, 0x0000000000000000ULL, DOWN},
			{0x00000000FF000000ULL, 0x000000FF00000000ULL, UP},
			{0x00000000FF000000ULL, 0x0000000000FF0000ULL, DOWN},
			{0x0101010101010101ULL, 0x0000000000000000ULL, LEFT},
			{0x0101010101010101ULL, 0x0202020202020202ULL, RIGHT},
			{0x8080808080808080ULL, 0x4040404040404040ULL, LEFT},
			{0x8080808080808080ULL, 0x0000000000000000ULL, RIGHT},
			{0x0808080808080808ULL, 0x0404040404040404ULL, LEFT},
			{0x0808080808080808ULL, 0x1010101010101010ULL, RIGHT},
		}};

		for (const auto& [input, expected, direction] : cases)
		{
			SECTION(std::to_string(input))
			{
				bitboard_t bb = input;
				bitboard_t shifted{};
				switch (direction)
				{
				case UP: shifted = shift<UP>(bb); break;
				case DOWN: shifted = shift<DOWN>(bb); break;
				case LEFT: shifted = shift<LEFT>(bb); break;
				case RIGHT: shifted = shift<RIGHT>(bb); break;
				}
				REQUIRE(shifted == expected);
			}
		}
	}
}
