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
#include "Loki/search/pv_table.hpp"

namespace pv_table_tests
{
	using namespace loki;
	using namespace loki::movegen;
	using namespace loki::position;
	using namespace loki::search;

	namespace
	{
		constexpr ply_t FIRST_PLY = static_cast<ply_t>(1);
		constexpr ply_t SECOND_PLY = static_cast<ply_t>(2);

		const move ROOT_MOVE{ E2, E4 , false};
		const move FIRST_MOVE{ E7, E5 , false};
		const move SECOND_MOVE{ G1, F3 , false};
		const move REPLACEMENT_ROOT_MOVE{ D2, D4, false };
		const move REPLACEMENT_FIRST_MOVE{ C7, C5 , false};
	}

	TEST_CASE("pv_table starts empty", "[search][pv_table]")
	{
		pv_table<> table;

		REQUIRE(table.get_pv(ROOT_PLY).empty());
		REQUIRE(table.get_pv(FIRST_PLY).empty());
		REQUIRE(table.begin() == table.end());
	}

	TEST_CASE("pv_table builds a triangular principal variation", "[search][pv_table]")
	{
		pv_table<> table;

		table.update_pv(SECOND_PLY, SECOND_MOVE);
		table.update_pv(FIRST_PLY, FIRST_MOVE);
		table.update_pv(ROOT_PLY, ROOT_MOVE);

		const std::vector<move> expected_root{ ROOT_MOVE, FIRST_MOVE, SECOND_MOVE };
		const std::vector<move> expected_first{ FIRST_MOVE, SECOND_MOVE };
		const std::vector<move> expected_second{ SECOND_MOVE };
		const std::vector<move> iterated_root(table.begin(), table.end());

		REQUIRE(table.get_pv(ROOT_PLY) == expected_root);
		REQUIRE(table.get_pv(FIRST_PLY) == expected_first);
		REQUIRE(table.get_pv(SECOND_PLY) == expected_second);
		REQUIRE(iterated_root == expected_root);
	}

	TEST_CASE("pv_table replacement snapshots the current successor line", "[search][pv_table]")
	{
		pv_table<> table;
		table.update_pv(SECOND_PLY, SECOND_MOVE);
		table.update_pv(FIRST_PLY, FIRST_MOVE);
		table.update_pv(ROOT_PLY, ROOT_MOVE);

		table.update_pv(FIRST_PLY, REPLACEMENT_FIRST_MOVE);

		const std::vector<move> original_root{ ROOT_MOVE, FIRST_MOVE, SECOND_MOVE };
		const std::vector<move> replacement_first{ REPLACEMENT_FIRST_MOVE, SECOND_MOVE };
		REQUIRE(table.get_pv(ROOT_PLY) == original_root);
		REQUIRE(table.get_pv(FIRST_PLY) == replacement_first);

		table.update_pv(ROOT_PLY, REPLACEMENT_ROOT_MOVE);

		const std::vector<move> rebuilt_root{
			REPLACEMENT_ROOT_MOVE,
			REPLACEMENT_FIRST_MOVE,
			SECOND_MOVE
		};
		REQUIRE(table.get_pv(ROOT_PLY) == rebuilt_root);
	}

	TEST_CASE("pv_table reset_for_ply clears only the selected line", "[search][pv_table]")
	{
		pv_table<> table;
		table.update_pv(SECOND_PLY, SECOND_MOVE);
		table.update_pv(FIRST_PLY, FIRST_MOVE);
		table.update_pv(ROOT_PLY, ROOT_MOVE);

		table.reset_for_ply(FIRST_PLY);

		const std::vector<move> original_root{ ROOT_MOVE, FIRST_MOVE, SECOND_MOVE };
		const std::vector<move> expected_second{ SECOND_MOVE };
		REQUIRE(table.get_pv(ROOT_PLY) == original_root);
		REQUIRE(table.get_pv(FIRST_PLY).empty());
		REQUIRE(table.get_pv(SECOND_PLY) == expected_second);

		table.update_pv(ROOT_PLY, REPLACEMENT_ROOT_MOVE);

		const std::vector<move> truncated_root{ REPLACEMENT_ROOT_MOVE };
		REQUIRE(table.get_pv(ROOT_PLY) == truncated_root);

		table.update_pv(FIRST_PLY, REPLACEMENT_FIRST_MOVE);
		table.update_pv(ROOT_PLY, REPLACEMENT_ROOT_MOVE);

		const std::vector<move> rebuilt_root{
			REPLACEMENT_ROOT_MOVE,
			REPLACEMENT_FIRST_MOVE,
			SECOND_MOVE
		};
		REQUIRE(table.get_pv(ROOT_PLY) == rebuilt_root);
	}

	TEST_CASE("pv_table clear empties all lines and permits reuse", "[search][pv_table]")
	{
		pv_table<> table;
		table.update_pv(SECOND_PLY, SECOND_MOVE);
		table.update_pv(FIRST_PLY, FIRST_MOVE);
		table.update_pv(ROOT_PLY, ROOT_MOVE);

		table.clear();

		REQUIRE(table.get_pv(ROOT_PLY).empty());
		REQUIRE(table.get_pv(FIRST_PLY).empty());
		REQUIRE(table.get_pv(SECOND_PLY).empty());
		REQUIRE(table.begin() == table.end());

		table.update_pv(ROOT_PLY, REPLACEMENT_ROOT_MOVE);

		const std::vector<move> reused_root{ REPLACEMENT_ROOT_MOVE };
		REQUIRE(table.get_pv(ROOT_PLY) == reused_root);
	}

	TEST_CASE("pv_table terminal specialization remains empty", "[search][pv_table]")
	{
		constexpr auto terminal_ply = static_cast<ply_t>(constants::MAX_DEPTH);
		pv_table<terminal_ply> table;

		table.update_pv(terminal_ply, ROOT_MOVE);
		REQUIRE(table.get_pv(terminal_ply).empty());
		REQUIRE(table.begin() == table.end());

		table.reset_for_ply(terminal_ply);
		table.clear();

		REQUIRE(table.get_pv(terminal_ply).empty());
		REQUIRE(table.begin() == table.end());
	}
}
