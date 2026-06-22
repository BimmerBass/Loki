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
#include "Loki/position/square.hpp"

namespace position_tests
{
	using namespace loki::position;

	struct square_case
	{
		e_rank rank;
		e_file file;
		e_square square_value;
		std::string_view algebraic;
	};

	static const std::array<square_case, 3> cases{{
		{RANK_1, FILE_A, A1, "A1"},
		{RANK_4, FILE_E, E4, "e4"},
		{RANK_8, FILE_H, H8, "H8"},
	}};

	TEST_CASE("square construction and accessors", "[position][square]")
	{
		for (const auto& test_case : cases)
		{
			SECTION(std::string{test_case.algebraic})
			{
				square sq1 = test_case.square_value;
				square sq2(test_case.rank, test_case.file);
				square sq3(std::string(test_case.algebraic));

				REQUIRE(sq1.value() == test_case.square_value);
				REQUIRE(sq2.value() == test_case.square_value);
				REQUIRE(sq3.value() == test_case.square_value);
				REQUIRE(sq1.rank() == test_case.rank);
				REQUIRE(sq1.file() == test_case.file);
			}
		}
	}

	TEST_CASE("square to_algebraic matches the expected casing", "[position][square]")
	{
		for (const auto& test_case : cases)
		{
			SECTION(std::string{test_case.algebraic})
			{
				square sq = test_case.square_value;
				REQUIRE(sq.to_algebraic() == std::string{static_cast<char>(std::tolower(static_cast<unsigned char>(test_case.algebraic[0])))} + std::to_string(static_cast<int>(test_case.rank) + 1));
			}
		}
	}

	TEST_CASE("increment operators support pre-increment", "[position][square]")
	{
		e_rank rank = RANK_1;

		REQUIRE(++rank == RANK_2);
		REQUIRE(rank == RANK_2);
		REQUIRE(rank-- == RANK_2);
		REQUIRE(rank == RANK_1);
	}

	TEST_CASE("square supports pre and post increment", "[position][square]")
	{
		square sq = A1;

		REQUIRE(++sq == square(B1));
		REQUIRE(sq == square(B1));
		REQUIRE(sq++ == square(B1));
		REQUIRE(sq == square(C1));
	}
}
