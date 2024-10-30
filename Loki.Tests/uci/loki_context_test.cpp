#include "pch.h"
#include "Loki/uci/loki_context.hpp"
#include "Loki/uci/loki_context.cpp"

class loki_context_test :
	public ::testing::Test
{
protected:
	loki_context_test() :
		output_stream{},
		context{ output_stream }
	{
	}

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
	ASSERT_THROW(context.debug(), loki::uci::loki_context::not_implemented_error);
}
TEST_F(loki_context_test, test_isready)
{
	ASSERT_EQ(output_stream.str(), "");
	context.isready();
	ASSERT_EQ(output_stream.str(), "readyok\n");
}
TEST_F(loki_context_test, test_setoption)
{
	ASSERT_THROW(context.setoption("", {}), loki::uci::loki_context::not_implemented_error);
}
TEST_F(loki_context_test, test_register)
{
	ASSERT_THROW(context.register_(), loki::uci::loki_context::not_implemented_error);
}
TEST_F(loki_context_test, test_ucinewgame)
{
	ASSERT_THROW(context.ucinewgame(), loki::uci::loki_context::not_implemented_error);
}
TEST_F(loki_context_test, test_position)
{
	ASSERT_THROW(context.position(nullptr), loki::uci::loki_context::not_implemented_error);
}
TEST_F(loki_context_test, test_go)
{
	ASSERT_THROW(context.go(nullptr), loki::uci::loki_context::not_implemented_error);
}
TEST_F(loki_context_test, test_stop)
{
	ASSERT_THROW(context.stop(), loki::uci::loki_context::not_implemented_error);
}
TEST_F(loki_context_test, test_ponderhit)
{
	ASSERT_THROW(context.ponderhit(), loki::uci::loki_context::not_implemented_error);
}