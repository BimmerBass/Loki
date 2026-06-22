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
#include "Loki/position/castle_rights.hpp"

namespace position_tests
{
	using namespace loki;
	using namespace loki::position;

	bool runtime_can_castle(const castle_rights& rights, side s, castling_direction d) noexcept
	{
		return (s == WHITE)
			? ((d == KINGSIDE) ? rights.can_castle<WHITE, KINGSIDE>() : rights.can_castle<WHITE, QUEENSIDE>())
			: ((d == KINGSIDE) ? rights.can_castle<BLACK, KINGSIDE>() : rights.can_castle<BLACK, QUEENSIDE>());
	}

	void runtime_set(castle_rights& rights, side s, castling_direction d, bool value) noexcept
	{
		if (s == WHITE)
		{
			if (d == KINGSIDE)
				rights.set<WHITE, KINGSIDE>(value);
			else
				rights.set<WHITE, QUEENSIDE>(value);
		}
		else
		{
			if (d == KINGSIDE)
				rights.set<BLACK, KINGSIDE>(value);
			else
				rights.set<BLACK, QUEENSIDE>(value);
		}
	}

	static const std::array<std::tuple<side, castling_direction>, 4> castle_cases{{
		{WHITE, KINGSIDE},
		{WHITE, QUEENSIDE},
		{BLACK, KINGSIDE},
		{BLACK, QUEENSIDE},
	}};

	TEST_CASE("castle rights default to false", "[position][castle_rights]")
	{
		castle_rights rights;
		REQUIRE_FALSE(rights.can_castle<WHITE, KINGSIDE>());
		REQUIRE_FALSE(rights.can_castle<WHITE, QUEENSIDE>());
		REQUIRE_FALSE(rights.can_castle<BLACK, KINGSIDE>());
		REQUIRE_FALSE(rights.can_castle<BLACK, QUEENSIDE>());
	}

	TEST_CASE("castle rights can be toggled independently", "[position][castle_rights]")
	{
		for (const auto& [side, direction] : castle_cases)
		{
			SECTION(std::string{side == WHITE ? "white" : "black"} + ":" + std::string{direction == KINGSIDE ? "kingside" : "queenside"})
			{
				castle_rights rights;
				REQUIRE_FALSE(runtime_can_castle(rights, side, direction));
				runtime_set(rights, side, direction, true);
				REQUIRE(runtime_can_castle(rights, side, direction));
				runtime_set(rights, side, direction, false);
				REQUIRE_FALSE(runtime_can_castle(rights, side, direction));
			}
		}
	}

	TEST_CASE("castle rights set one value leaves the others untouched", "[position][castle_rights]")
	{
		castle_rights rights;
		rights.set<WHITE, KINGSIDE>(true);

		REQUIRE_FALSE(rights.can_castle<WHITE, QUEENSIDE>());
		REQUIRE_FALSE(rights.can_castle<BLACK, KINGSIDE>());
		REQUIRE_FALSE(rights.can_castle<BLACK, QUEENSIDE>());
	}

	TEST_CASE("castle rights stringify empty and full states", "[position][castle_rights]")
	{
		castle_rights rights;
		REQUIRE(rights.to_string() == "-");

		rights.set<WHITE, QUEENSIDE>(true);
		rights.set<BLACK, KINGSIDE>(true);
		REQUIRE(rights.to_string() == "Qk");

		rights.set<WHITE, KINGSIDE>(true);
		rights.set<BLACK, QUEENSIDE>(true);
		REQUIRE(rights.to_string() == "KQkq");
	}
}
