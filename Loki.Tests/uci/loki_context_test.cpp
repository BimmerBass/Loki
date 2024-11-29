#include "pch.hpp"
#include "Loki/uci/loki_context.hpp"
#include "Loki/uci/loki_context.cpp"

namespace uci_tests
{

	class loki_context_test :
		public ::testing::Test
	{
	protected:
		loki_context_test() :
			output_stream{},
			context{ output_stream }
		{}

		std::stringstream output_stream;
		loki::uci::loki_context context;
	};

	std::vector<std::string> split_on(const std::string& str, char sep)
	{
		auto rng = str
			| std::views::split(sep)
			| std::views::filter([](const auto& sr) { return std::distance(sr.begin(), sr.end()) > 0; })
			| std::views::transform([](const auto& sr) { return std::string(sr.begin(), sr.end()); });
		return std::vector(rng.begin(), rng.end());
	}

	TEST_F(loki_context_test, test_uci)
	{
		ASSERT_EQ(output_stream.str(), "");
		context.uci();

		std::string output = output_stream.str();
		char linebreak = '\n';
		auto lines = split_on(output, linebreak);

		ASSERT_EQ(lines.size(), 3);
		ASSERT_TRUE(lines[0].substr(0, 8) == "id name ");
		ASSERT_TRUE(lines[1].substr(0, 10) == "id author ");
		ASSERT_EQ(lines[2], "uciok");
	}
	TEST_F(loki_context_test, test_debug)
	{
		ASSERT_THROW(context.debug(), loki::not_implemented_error);
	}
	TEST_F(loki_context_test, test_isready)
	{
		ASSERT_EQ(output_stream.str(), "");
		context.isready();
		ASSERT_EQ(output_stream.str(), "readyok\n");
	}
	TEST_F(loki_context_test, test_setoption)
	{
		ASSERT_THROW(context.setoption("", {}), loki::not_implemented_error);
	}
	TEST_F(loki_context_test, test_register)
	{
		ASSERT_THROW(context.register_(), loki::not_implemented_error);
	}
	TEST_F(loki_context_test, test_ucinewgame)
	{
		ASSERT_THROW(context.ucinewgame(), loki::not_implemented_error);
	}
	TEST_F(loki_context_test, test_position)
	{
		auto ev = std::vector<std::string>();
		ASSERT_EQ(context.game_state(), nullptr);
		context.position(loki::constants::START_FEN, ev);
		ASSERT_NE(context.game_state(), nullptr);
	}
	TEST_F(loki_context_test, test_go)
	{
		ASSERT_THROW(context.go(nullptr), loki::not_implemented_error);
	}
	TEST_F(loki_context_test, test_stop)
	{
		ASSERT_THROW(context.stop(), loki::not_implemented_error);
	}
	TEST_F(loki_context_test, test_ponderhit)
	{
		ASSERT_THROW(context.ponderhit(), loki::not_implemented_error);
	}
}