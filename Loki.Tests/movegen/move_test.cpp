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
#include "Loki/movegen/move.hpp"

namespace move_tests
{
	using namespace loki::movegen;
	using namespace loki::position;
	using namespace loki;

	TEST_CASE("move construction from raw values", "[movegen][move]")
	{
		SECTION("all zero")
		{
			move zero(move_t{0});
			REQUIRE(zero.from() == A1);
			REQUIRE(zero.to() == A1);
			REQUIRE(zero.type() == NORMAL);
			REQUIRE(zero.promotion_piece() == KNIGHT);
		}

		SECTION("mixed value")
		{
			const auto random_move = move_t(((uint16_t)32 << 10) | ((uint16_t)31 << 4) | (CASTLING << 2) | (ROOK - 1));
			move random(random_move);
			REQUIRE(random.from() == A5);
			REQUIRE(random.to() == H4);
			REQUIRE(random.type() == CASTLING);
			REQUIRE(random.promotion_piece() == ROOK);
		}

		SECTION("all bits set")
		{
			const move_t full_move = move_t(((uint16_t)63 << 10) | ((uint16_t)63 << 4) | (PROMOTION << 2) | (QUEEN - 1));
			move full(full_move);
			REQUIRE(full.from() == H8);
			REQUIRE(full.to() == H8);
			REQUIRE(full.type() == PROMOTION);
			REQUIRE(full.promotion_piece() == QUEEN);
		}
	}

	TEST_CASE("move setters update the underlying value", "[movegen][move]")
	{
		SECTION("from")
		{
			move m((move_t)0);
			REQUIRE(m.from() == A1);
			m.from(H8);
			REQUIRE(m.from() == H8);
		}

		SECTION("to")
		{
			move m((move_t)0);
			REQUIRE(m.to() == A1);
			m.to(H8);
			REQUIRE(m.to() == H8);
		}

		SECTION("type")
		{
			move m((move_t)0);
			REQUIRE(m.type() == NORMAL);
			m.type(CASTLING);
			REQUIRE(m.type() == CASTLING);
		}

		SECTION("promotion piece")
		{
			move m((move_t)0);
			REQUIRE(m.promotion_piece() == KNIGHT);
			m.promotion_piece(ROOK);
			REQUIRE(m.promotion_piece() == ROOK);
		}
	}
}
