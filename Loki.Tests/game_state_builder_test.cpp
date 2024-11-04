#include "pch.hpp"
#include "Loki/util/stringops.hpp"
#include "Loki/position/io/game_state_builder.hpp"
#include "Loki/position/io/game_state_builder.cpp"
#include "Loki/position/game_state.hpp"


namespace position_tests::io_tests
{
	using namespace loki;
	using namespace loki::position;
	using namespace loki::position::io;
	namespace fs = std::filesystem;

	struct test_fen
	{
		std::string fen;
		std::string piece_placements;
		std::string side_to_move;
		std::string castling_abilities;
		std::string en_passant_sq;
		std::string halfmove_clock;
		std::string fullmove_clock;
	};

	class game_state_builder_ctor_test :
		public ::testing::Test
	{
	public:
		game_state_builder_ctor_test()
		{
			auto path = fs::path(get_project_dir()) / "fens.epd";
			assert(fs::exists(path));

			auto file = std::ifstream(path);
			auto firstLine = true;
			std::string line;
			while (std::getline(file, line))
			{
				if (firstLine)
				{
					firstLine = false;
					continue;
				}
				auto splitted = loki::util::split(line, '\t');
				assert(splitted.size() == 7);
				fens.push_back(test_fen{
					splitted[0],
					splitted[1],
					splitted[2],
					splitted[3],
					splitted[4],
					splitted[5],
					splitted[6]
				});
			}
		}

		std::vector<test_fen> fens;
	};

#pragma region RESET
	TEST_F(game_state_builder_ctor_test, test_constructor)
	{
		game_state_builder builder;
		auto gamestate = std::make_shared<loki::position::game_state>();

		for (auto& test_case : fens)
		{
			auto sPtr = std::shared_ptr<std::string>(new std::string(test_case.fen));
			builder.reset(sPtr, gamestate);

			EXPECT_EQ(builder.get_piece_placements(), test_case.piece_placements);
			EXPECT_EQ(builder.get_side_to_move(), test_case.side_to_move[0]);
			EXPECT_EQ(builder.get_castling_abilities(), test_case.castling_abilities);
			EXPECT_EQ(builder.get_en_passant_sq(), test_case.en_passant_sq);
			EXPECT_EQ(builder.get_halfmove_clock(), test_case.halfmove_clock);
			EXPECT_EQ(builder.get_fullmove_clock(), test_case.fullmove_clock);
		}
	}

	TEST(game_state_builder_test, test_invalid_fen)
	{
		game_state_builder builder;
		auto gamestate = std::make_shared<loki::position::game_state>();

		EXPECT_THROW(builder.reset(std::make_shared<std::string>(""), gamestate), game_state_builder::fen_parsing_error);
		EXPECT_THROW(builder.reset(std::make_shared<std::string>("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w"), gamestate), game_state_builder::fen_parsing_error);
	}
#pragma endregion

	std::array<std::array<char, 8>, 8> simple_piece_placements_parse(std::string pps)
	{
		auto rows = loki::util::split(pps, '/');
		std::reverse(rows.begin(), rows.end());
		std::array<std::array<char, 8>, 8> parsed = { };
		
		for (int r = 0; r < 8; r++)
		{
			parsed[r].fill('-');
			int f = 0;
			for (auto& c : rows[r])
			{
				if (std::isdigit(c))
					f += c - '0';
				else
					parsed[r][f++] = c;
			}
		}
		return parsed;
	}

	static std::map<char, piece> piece_name_mapping = {
		{'p', PAWN},
		{'n', KNIGHT},
		{'b', BISHOP},
		{'r', ROOK},
		{'q', QUEEN},
		{'k', KING}
	};

	TEST_F(game_state_builder_ctor_test, test_piece_placements)
	{
		game_state_builder builder;
		auto gamestate = std::make_shared<loki::position::game_state>();

		for (auto& test_case : fens)
		{
			auto sPtr = std::shared_ptr<std::string>(new std::string(test_case.fen));
			builder.reset(sPtr, gamestate)
				.piece_placements();

			auto simply_parsed = simple_piece_placements_parse(test_case.piece_placements);
			for (square sq = A1; sq <= H8; sq++)
			{
				auto current = simply_parsed[sq.rank()][sq.file()];
				if (current == '-')
				{
					EXPECT_EQ(gamestate->piece_placements[WHITE][sq.value()], NO_PIECE);
					EXPECT_EQ(gamestate->piece_placements[BLACK][sq.value()], NO_PIECE);
				}
				else if (std::isupper(current)) // white
				{
					current = std::tolower(current);
					EXPECT_EQ(gamestate->piece_placements[WHITE][sq.value()], piece_name_mapping[current]);
					EXPECT_EQ(gamestate->piece_placements[BLACK][sq.value()], NO_PIECE);
				}
				else { // black
					EXPECT_EQ(gamestate->piece_placements[BLACK][sq.value()], piece_name_mapping[current]);
					EXPECT_EQ(gamestate->piece_placements[WHITE][sq.value()], NO_PIECE);
				}
			}
		}
	}

	TEST_F(game_state_builder_ctor_test, test_side_to_move)
	{
		game_state_builder builder;
		auto gamestate = std::make_shared<loki::position::game_state>();
		
		for (auto& test_case : fens)
		{
			auto sPtr = std::shared_ptr<std::string>(new std::string(test_case.fen));
			builder.reset(sPtr, gamestate)
				.side_to_move();

			if (test_case.side_to_move == "w")
				EXPECT_EQ(gamestate->side_to_move, loki::WHITE);
			else
				EXPECT_EQ(gamestate->side_to_move, loki::BLACK);
		}
	}

	TEST_F(game_state_builder_ctor_test, test_castling_abilities)
	{
		game_state_builder builder;
		auto gamestate = std::make_shared<loki::position::game_state>();

		for (auto& test_case : fens)
		{
			auto sPtr = std::shared_ptr<std::string>(new std::string(test_case.fen));
			builder.reset(sPtr, gamestate)
				.castling_ability();

			auto& c = gamestate->castling_rights;
			auto& cas = test_case.castling_abilities;
			if (cas == "-")
			{
				EXPECT_FALSE((
					c.can_castle<WHITE, KINGSIDE>() ||
					c.can_castle<WHITE, QUEENSIDE>() ||
					c.can_castle<BLACK, KINGSIDE>() ||
					c.can_castle<BLACK, QUEENSIDE>()));
			}
			else
			{
				if (cas.find("K") != std::string::npos)
					EXPECT_TRUE((c.can_castle<WHITE, KINGSIDE>()));
				if (cas.find("Q") != std::string::npos)
					EXPECT_TRUE((c.can_castle<WHITE, QUEENSIDE>()));
				if (cas.find("k") != std::string::npos)
					EXPECT_TRUE((c.can_castle<BLACK, KINGSIDE>()));
				if (cas.find("q") != std::string::npos)
					EXPECT_TRUE((c.can_castle<BLACK, QUEENSIDE>()));
			}
		}
	}

	TEST_F(game_state_builder_ctor_test, test_en_passant_square)
	{
		game_state_builder builder;
		auto gamestate = std::make_shared<loki::position::game_state>();

		for (auto& test_case : fens)
		{
			auto sPtr = std::shared_ptr<std::string>(new std::string(test_case.fen));
			builder.reset(sPtr, gamestate)
				.en_passant_square();

			if (test_case.en_passant_sq != "-")
			{
				loki::position::square ep_sq = test_case.en_passant_sq;
				EXPECT_EQ(gamestate->en_passant_sq, ep_sq.value());
			}
			else
				EXPECT_EQ(gamestate->en_passant_sq, loki::position::NO_SQ);
		}
	}

	TEST_F(game_state_builder_ctor_test, test_clocks)
	{
		game_state_builder builder;
		auto gamestate = std::make_shared<loki::position::game_state>();

		for (auto& test_case : fens)
		{
			auto sPtr = std::shared_ptr<std::string>(new std::string(test_case.fen));
			builder.reset(sPtr, gamestate)
				.halfmove_clock()
				.fullmove_clock();

			EXPECT_EQ(gamestate->fifty_move_cnt, std::stoi(test_case.halfmove_clock));
			EXPECT_EQ(gamestate->full_move_cnt, std::stoi(test_case.fullmove_clock));
		}
	}
}