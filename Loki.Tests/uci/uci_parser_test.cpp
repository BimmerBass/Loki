#include "pch.hpp"
#include "Loki/uci/uci_parser.hpp"
#include "Loki/uci/uci_parser.cpp"
#include "Loki/position/io/fen_string_builder.cpp"
#include "mocks/context_mock.hpp"
using namespace loki::uci;

namespace uci_tests
{
	class uci_parser_mocked :
		public ::testing::Test
	{
	public:
		uci_parser_mocked()
		{
			context = std::make_shared<context_mock>();
		}

		inline static const std::vector<std::string> no_tokens = {};
		std::shared_ptr<context_mock> context;
	};

	
	template<typename Base, typename Derived>
	std::unique_ptr<Base> static_downcast(std::unique_ptr<Derived>& ptr)
	{
		auto rawPtr = static_cast<Base*>(ptr.release());
		return std::unique_ptr<Base>(rawPtr);
	}

	TEST_F(uci_parser_mocked, test_uci)
	{
		using namespace ::testing;
		EXPECT_CALL(*context, uci)
			.Times(1);
		uci_parser parser(context);
		parser.parse_uci(no_tokens);
	}
	TEST_F(uci_parser_mocked, test_isready)
	{
		using namespace ::testing;
		EXPECT_CALL(*context, isready)
			.Times(1);
		uci_parser parser(context);
		parser.parse_isready(no_tokens);
	}

#pragma region SETOPTION
	TEST_F(uci_parser_mocked, test_setoption_no_tokens_or_invalid)
	{
		using namespace ::testing;
		EXPECT_CALL(*context, setoption)
			.Times(0);
		uci_parser parser(context);

		EXPECT_THROW(parser.parse_setoption(no_tokens), uci_parser::uci_error);
		EXPECT_THROW(parser.parse_setoption({ "hello", "world" }), uci_parser::uci_error);
	}
	TEST_F(uci_parser_mocked, test_setoption_valid_novalue)
	{
		using namespace ::testing;
		EXPECT_CALL(*context, setoption("testoption", std::optional<std::string>(std::nullopt)))
			.Times(1);
		uci_parser parser(context);
		parser.parse_setoption({ "name", "testOption" });
	}
	TEST_F(uci_parser_mocked, test_setoption_valid_withvalue)
	{
		using namespace ::testing;
		EXPECT_CALL(*context, setoption("testoption", std::optional<std::string>("testvalue")))
			.Times(1);
		uci_parser parser(context);
		parser.parse_setoption({ "name", "testOption", "value", "testValue"});
	}
#pragma endregion
#pragma region POSITION	
	TEST_F(uci_parser_mocked, test_position_empty)
	{
		using namespace ::testing;
		EXPECT_CALL(*context, position)
			.Times(0);
		uci_parser parser(context);
		EXPECT_THROW(parser.parse_position(no_tokens), uci_parser::uci_error);
	}
	TEST_F(uci_parser_mocked, test_position_startpos)
	{
		using namespace ::testing;
		EXPECT_CALL(*context, position(loki::constants::START_FEN, std::vector<std::string>()))
			.Times(1);
		uci_parser parser(context);
		parser.parse_position({"startpos"});
	}
	TEST_F(uci_parser_mocked, test_position_fen_moves)
	{
		using namespace ::testing;
		auto test_fen = "FEN: 8/8/8/4p1K1/2k1P3/8/8/8 b - - 0 1";
		EXPECT_CALL(*context, position(test_fen, std::vector<std::string> {"e2e4", "e7e5" }))
			.Times(1);
		uci_parser parser(context);
		parser.parse_position({ "fen", test_fen, "moves", "e2e4", "e7e5" });
	}
#pragma endregion
#pragma region GO
	// TODO: Finish these tests when we get to actually parsing the go-parameters
	TEST_F(uci_parser_mocked, test_go_empty) // infinite
	{
		using namespace ::testing;
		EXPECT_CALL(*context, go)
			.Times(1);
		uci_parser parser(context);
		parser.parse_go(no_tokens);
	}
#pragma endregion

	TEST_F(uci_parser_mocked, test_ucinewgame)
	{
		using namespace ::testing;
		EXPECT_CALL(*context, ucinewgame)
			.Times(1);
		uci_parser parser(context);
		parser.parse_ucinewgame(no_tokens);
	}
	TEST_F(uci_parser_mocked, test_stop)
	{
		using namespace ::testing;
		EXPECT_CALL(*context, stop)
			.Times(1);
		uci_parser parser(context);
		parser.parse_stop(no_tokens);
	}

	TEST_F(uci_parser_mocked, test_ponderhit)
	{
		using namespace ::testing;
		EXPECT_CALL(*context, ponderhit)
			.Times(1);
		uci_parser parser(context);
		parser.parse_ponderhit(no_tokens);
	}
}