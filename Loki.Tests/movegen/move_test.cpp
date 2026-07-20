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
			const move_t raw_move{ 0 };
			move zero(raw_move);
			REQUIRE(zero.from() == A1);
			REQUIRE(zero.to() == A1);
			REQUIRE(zero.type() == NORMAL);
			REQUIRE(zero.promotion_piece() == KNIGHT);
			REQUIRE(zero.get_move() == raw_move);
			REQUIRE_FALSE(zero.is_active());
			REQUIRE(zero.score() == 0);
		}

		SECTION("mixed value")
		{
			const auto random_move = move_t(((uint16_t)32 << 10) | ((uint16_t)31 << 4) | (CASTLING << 2) | (ROOK - 1));
			move random(random_move);
			REQUIRE(random.from() == A5);
			REQUIRE(random.to() == H4);
			REQUIRE(random.type() == CASTLING);
			REQUIRE(random.promotion_piece() == ROOK);
			REQUIRE(random.get_move() == random_move);
			REQUIRE_FALSE(random.is_active());
			REQUIRE(random.score() == 0);
		}

		SECTION("all bits set")
		{
			const move_t full_move = move_t(((uint16_t)63 << 10) | ((uint16_t)63 << 4) | (PROMOTION << 2) | (QUEEN - 1));
			move full(full_move);
			REQUIRE(full.from() == H8);
			REQUIRE(full.to() == H8);
			REQUIRE(full.type() == PROMOTION);
			REQUIRE(full.promotion_piece() == QUEEN);
			REQUIRE(full.get_move() == full_move);
			REQUIRE_FALSE(full.is_active());
			REQUIRE(full.score() == 0);
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

	TEST_CASE("move activity flag can be set without changing other encoded values", "[movegen][move]")
	{
		move m(A5, H4, CASTLING, ROOK, true, 12'345);
		const auto raw_move = m.get_move();

		REQUIRE(m.is_active());
		REQUIRE(m.score() == 12'345);

		m.active(false);
		REQUIRE_FALSE(m.is_active());
		REQUIRE(m.score() == 12'345);
		REQUIRE(m.get_move() == raw_move);

		m.active(true);
		REQUIRE(m.is_active());
		REQUIRE(m.score() == 12'345);
		REQUIRE(m.get_move() == raw_move);
	}

	TEST_CASE("move score can be set without changing other encoded values", "[movegen][move]")
	{
		move m(A5, H4, CASTLING, ROOK, true);
		const auto raw_move = m.get_move();

		REQUIRE(m.score() == 0);

		m.score(1);
		REQUIRE(m.score() == 1);
		REQUIRE(m.is_active());
		REQUIRE(m.get_move() == raw_move);

		m.score(12'345);
		REQUIRE(m.score() == 12'345);
		REQUIRE(m.is_active());
		REQUIRE(m.get_move() == raw_move);

		m.score(-12'345);
		REQUIRE(m.score() == -12'345);
		REQUIRE(m.is_active());
		REQUIRE(m.get_move() == raw_move);

		m.score(0x3FFF);
		REQUIRE(m.score() == 0x3FFF);
		REQUIRE(m.is_active());
		REQUIRE(m.get_move() == raw_move);

		m.score(-0x4000);
		REQUIRE(m.score() == -0x4000);
		REQUIRE(m.is_active());
		REQUIRE(m.get_move() == raw_move);
	}

	TEST_CASE("move score rejects values that do not fit in a signed 15-bit value", "[movegen][move]")
	{
		move m(move_t{ 0 });

		REQUIRE_THROWS(m.score(0x4000));
		REQUIRE_THROWS(m.score(-0x4001));
	}

	TEST_CASE("move constructor keeps move, activity, and score fields independent", "[movegen][move]")
	{
		const auto expected_move = move_t(
			((uint16_t)32 << 10)
			| ((uint16_t)31 << 4)
			| (CASTLING << 2)
			| (ROOK - 1));
		move m(A5, H4, CASTLING, ROOK, true, -0x4000);

		REQUIRE(m.from() == A5);
		REQUIRE(m.to() == H4);
		REQUIRE(m.type() == CASTLING);
		REQUIRE(m.promotion_piece() == ROOK);
		REQUIRE(m.is_active());
		REQUIRE(m.score() == -0x4000);
		REQUIRE(m.get_move() == expected_move);
	}
}
