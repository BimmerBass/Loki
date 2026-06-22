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
#include "fen_reader.hpp"
#include "Loki/position/io/game_state_builder.hpp"
#include "Loki/position/game_state.hpp"

namespace position_tests::io_tests
{
	using namespace loki;
	using namespace loki::position;
	using namespace loki::position::io;

	std::array<std::array<char, 8>, 8> simple_piece_placements_parse(std::string piece_placements)
	{
		auto rows = loki::util::split(piece_placements, '/');
		std::reverse(rows.begin(), rows.end());
		std::array<std::array<char, 8>, 8> parsed{};

		for (int rank = 0; rank < 8; ++rank)
		{
			parsed[rank].fill('-');
			int file = 0;
			for (const auto c : rows[rank])
			{
				if (std::isdigit(static_cast<unsigned char>(c)))
					file += c - '0';
				else
					parsed[rank][file++] = c;
			}
		}
		return parsed;
	}

	static const std::map<char, piece> piece_name_mapping{
		{'p', PAWN},
		{'n', KNIGHT},
		{'b', BISHOP},
		{'r', ROOK},
		{'q', QUEEN},
		{'k', KING},
	};

	class game_state_builder_fixture : public fen_reader
	{
	};

	TEST_CASE_METHOD(game_state_builder_fixture, "game_state_builder parses the shared FEN corpus", "[position][fen][builder]")
	{
		game_state_builder builder;
		auto gamestate = std::make_shared<game_state>();

		for (const auto& test_case : fens)
		{
			SECTION(test_case.fen)
			{
				auto fen = std::make_shared<std::string>(test_case.fen);
				builder.reset(fen, gamestate);

				REQUIRE(builder.get_piece_placements() == test_case.piece_placements);
				REQUIRE(builder.get_side_to_move() == test_case.side_to_move[0]);
				REQUIRE(builder.get_castling_abilities() == test_case.castling_abilities);
				REQUIRE(builder.get_en_passant_sq() == test_case.en_passant_sq);
				REQUIRE(builder.get_halfmove_clock() == test_case.halfmove_clock);
				REQUIRE(builder.get_fullmove_clock() == test_case.fullmove_clock);
			}
		}
	}

	TEST_CASE("game_state_builder rejects invalid FEN strings", "[position][fen][builder]")
	{
		game_state_builder builder;
		auto gamestate = std::make_shared<game_state>();
		REQUIRE_THROWS_AS(builder.reset(std::make_shared<std::string>(""), gamestate), game_state_builder::fen_parsing_error);
		REQUIRE_THROWS_AS(
			builder.reset(std::make_shared<std::string>("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w"), gamestate),
			game_state_builder::fen_parsing_error);
	}

	TEST_CASE_METHOD(game_state_builder_fixture, "game_state_builder parses piece placement correctly", "[position][fen][builder]")
	{
		game_state_builder builder;
		auto gamestate = std::make_shared<game_state>();

		for (const auto& test_case : fens)
		{
			SECTION(test_case.fen)
			{
				auto fen = std::make_shared<std::string>(test_case.fen);
				builder.reset(fen, gamestate).piece_placements();

				const auto parsed = simple_piece_placements_parse(test_case.piece_placements);
				for (square sq = A1; sq <= H8; ++sq)
				{
					const auto current = parsed[sq.rank()][sq.file()];
					if (current == '-')
					{
						REQUIRE(gamestate->piece_placements[WHITE][sq.value()] == NO_PIECE);
						REQUIRE(gamestate->piece_placements[BLACK][sq.value()] == NO_PIECE);
					}
					else if (std::isupper(static_cast<unsigned char>(current)))
					{
						const auto lower = static_cast<char>(std::tolower(static_cast<unsigned char>(current)));
						REQUIRE(gamestate->piece_placements[WHITE][sq.value()] == piece_name_mapping.at(lower));
						REQUIRE(gamestate->piece_placements[BLACK][sq.value()] == NO_PIECE);
					}
					else
					{
						REQUIRE(gamestate->piece_placements[BLACK][sq.value()] == piece_name_mapping.at(current));
						REQUIRE(gamestate->piece_placements[WHITE][sq.value()] == NO_PIECE);
					}
				}
			}
		}
	}

	TEST_CASE_METHOD(game_state_builder_fixture, "game_state_builder parses side-to-move", "[position][fen][builder]")
	{
		game_state_builder builder;
		auto gamestate = std::make_shared<game_state>();

		for (const auto& test_case : fens)
		{
			SECTION(test_case.fen)
			{
				auto fen = std::make_shared<std::string>(test_case.fen);
				builder.reset(fen, gamestate).side_to_move();

				if (test_case.side_to_move == "w")
					REQUIRE(gamestate->side_to_move == WHITE);
				else
					REQUIRE(gamestate->side_to_move == BLACK);
			}
		}
	}

	TEST_CASE_METHOD(game_state_builder_fixture, "game_state_builder parses castling rights", "[position][fen][builder]")
	{
		game_state_builder builder;
		auto gamestate = std::make_shared<game_state>();

		for (const auto& test_case : fens)
		{
			SECTION(test_case.fen)
			{
				auto fen = std::make_shared<std::string>(test_case.fen);
				builder.reset(fen, gamestate).castling_ability();

				const auto& rights = gamestate->castling_rights;
				if (test_case.castling_abilities == "-")
				{
					REQUIRE_FALSE(rights.can_castle<WHITE, KINGSIDE>());
					REQUIRE_FALSE(rights.can_castle<WHITE, QUEENSIDE>());
					REQUIRE_FALSE(rights.can_castle<BLACK, KINGSIDE>());
					REQUIRE_FALSE(rights.can_castle<BLACK, QUEENSIDE>());
				}
				else
				{
					if (test_case.castling_abilities.find('K') != std::string::npos)
						REQUIRE(rights.can_castle<WHITE, KINGSIDE>());
					if (test_case.castling_abilities.find('Q') != std::string::npos)
						REQUIRE(rights.can_castle<WHITE, QUEENSIDE>());
					if (test_case.castling_abilities.find('k') != std::string::npos)
						REQUIRE(rights.can_castle<BLACK, KINGSIDE>());
					if (test_case.castling_abilities.find('q') != std::string::npos)
						REQUIRE(rights.can_castle<BLACK, QUEENSIDE>());
				}
			}
		}
	}

	TEST_CASE_METHOD(game_state_builder_fixture, "game_state_builder parses en-passant squares", "[position][fen][builder]")
	{
		game_state_builder builder;
		auto gamestate = std::make_shared<game_state>();

		for (const auto& test_case : fens)
		{
			SECTION(test_case.fen)
			{
				auto fen = std::make_shared<std::string>(test_case.fen);
				builder.reset(fen, gamestate).en_passant_square();

				if (test_case.en_passant_sq != "-")
				{
					square ep_sq = test_case.en_passant_sq;
					REQUIRE(gamestate->en_passant_sq == ep_sq.value());
				}
				else
				{
					REQUIRE(gamestate->en_passant_sq == loki::position::NO_SQ);
				}
			}
		}
	}

	TEST_CASE_METHOD(game_state_builder_fixture, "game_state_builder parses clocks", "[position][fen][builder]")
	{
		game_state_builder builder;
		auto gamestate = std::make_shared<game_state>();

		for (const auto& test_case : fens)
		{
			SECTION(test_case.fen)
			{
				auto fen = std::make_shared<std::string>(test_case.fen);
				builder.reset(fen, gamestate).halfmove_clock().fullmove_clock();

				REQUIRE(gamestate->fifty_move_cnt == std::stoi(test_case.halfmove_clock));
				REQUIRE(gamestate->full_move_cnt == std::stoi(test_case.fullmove_clock));
			}
		}
	}
}
