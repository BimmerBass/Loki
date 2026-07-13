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
#include "Loki/position/game_state.hpp"
#include "Loki/position/io/game_state_builder.hpp"
#include "io/fen_reader.hpp"

namespace position_tests
{
	using namespace loki;
	using namespace loki::position;
	using namespace loki::position::io;

	class game_state_fixture : public io_tests::fen_reader
	{
	};

	TEST_CASE("game_state rejects empty builder input", "[position][game_state][fen]")
	{
		game_state_builder builder;
		REQUIRE_THROWS_AS(game_state::from_builder(&builder, ""), io::game_state_builder::fen_parsing_error);
	}

	TEST_CASE_METHOD(game_state_fixture, "game_state round-trips the shared FEN corpus", "[position][game_state][fen]")
	{
		for (const auto& test_case : fens)
		{
			SECTION(test_case.fen)
			{
				const auto gs = game_state::from_fen(test_case.fen);
				const auto fen = game_state::to_fen(gs);
				REQUIRE(fen == test_case.fen);
			}
		}
	}

	TEST_CASE("game_state get_piece returns the correct side and piece", "[position][game_state]")
	{
		game_state gs;
		gs.piece_placements[WHITE].fill(NO_PIECE);
		gs.piece_placements[BLACK].fill(NO_PIECE);

		SECTION("empty square")
		{
			side s = WHITE;
			REQUIRE(gs.get_piece(A1, &s) == NO_PIECE);
			REQUIRE(s == NUM_SIDES);
		}

		SECTION("white piece")
		{
			side s = WHITE;
			gs.piece_placements[WHITE][B7] = KNIGHT;
			REQUIRE(gs.get_piece(B7, &s) == KNIGHT);
			REQUIRE(s == WHITE);
		}

		SECTION("black piece")
		{
			side s = WHITE;
			gs.piece_placements[BLACK][F6] = QUEEN;
			REQUIRE(gs.get_piece(F6, &s) == QUEEN);
			REQUIRE(s == BLACK);
		}

		SECTION("both sides on one square throws")
		{
			side s = WHITE;
			gs.piece_placements[BLACK][F6] = QUEEN;
			gs.piece_placements[WHITE][F6] = KING;
			REQUIRE_THROWS_AS(gs.get_piece(F6, &s), loki_exception);
		}
	}

	TEST_CASE_METHOD(game_state_fixture, "game_state::flip_fen returns flipped fen", "[position][game_state]")
	{
		SECTION("single fen gets mirrored properly")
		{
			auto fen = "r2q1rk1/pp3ppp/1bpN1n2/4n3/1PP1P1b1/P1N2PP1/6BP/R1BQK2R b KQ - 60 30";
			auto flipped = "r1bqk2r/6bp/p1n2pp1/1pp1p1B1/4N3/1BPn1N2/PP3PPP/R2Q1RK1 w kq - 60 30";
			REQUIRE(game_state::flip_fen(fen) == flipped);
		}

		SECTION("en passant rank is mirrored as a digit")
		{
			auto fen = "8/8/8/8/8/8/8/8 w - f6 0 1";
			auto flipped = "8/8/8/8/8/8/8/8 b - f3 0 1";
			REQUIRE(game_state::flip_fen(fen) == flipped);
		}

		SECTION("fen flipping is symmetrical")
		{
			for (const auto& test_case : fens)
			{
				SECTION(test_case.fen)
				{
					auto flipped_case = game_state::flip_fen(test_case.fen);
					REQUIRE(game_state::flip_fen(flipped_case) == test_case.fen);
				}
			}
		}
	}
}
