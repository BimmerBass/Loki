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
#include "Loki/movegen/move_list.hpp"

namespace move_tests
{
	using namespace loki::movegen;
	using namespace loki;

	TEST_CASE("move_list push_back clear and iteration", "[movegen][move_list]")
	{
		SECTION("push_back fills the list up to capacity")
		{
			move_list ml;
			for (size_t i = 0; i < ml.max_size; ++i)
			{
				REQUIRE(ml.size() == i);
				ml.push_back(move_t(i + 1));
				REQUIRE(ml[i].get_move() == i + 1);
				if (i > 0)
					REQUIRE(ml[i - 1].get_move() == i);
				if (i < ml.max_size - 1)
					REQUIRE_THROWS_AS(ml[i + 1].get_move(), move_list::move_list_error);
			}
			REQUIRE(ml.size() == ml.max_size);
			REQUIRE_THROWS_AS(ml.push_back(move_t(0)), move_list::move_list_error);
		}

		SECTION("clear resets the size")
		{
			move_list ml;
			ml.push_back(move_t(0));
			ml.push_back(move_t(0));
			REQUIRE(ml.size() == 2);
			ml.clear();
			REQUIRE(ml.size() == 0);
		}

		SECTION("iteration yields inserted elements in order")
		{
			move_list ml;
			for (size_t i = 0; i < ml.max_size / 2; ++i)
				ml.push_back(move_t(i + 1));

			size_t expected = 1;
			for (const auto& move : ml)
			{
				REQUIRE(move.get_move() == expected);
				++expected;
			}
		}
	}
}
