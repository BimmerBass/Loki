#include "pch.hpp"
#include "loki/util/stringops.hpp"
#include "loki/util/stringops.cpp"

namespace util_tests
{
	using namespace loki::util;

	/* SPLIT */
	TEST(stringops_test, split_empty_str)
	{
		ASSERT_EQ(split("", '\t'), std::vector<std::string>());
	}
	TEST(stringops_test, split_no_sep_present)
	{
		ASSERT_EQ(split("hello, world!", '|'), std::vector<std::string> { "hello, world!" });
	}
	TEST(stringops_test, split_sep_present_keepempty)
	{
		auto v = std::vector<std::string>{ "he", "", "o, wor", "d!" };
		ASSERT_EQ(split("hello, world!", 'l', true), v);
	}
	TEST(stringops_test, split_sep_present_removeempty)
	{
		auto v = std::vector<std::string>{ "he", "o, wor", "d!" };
		ASSERT_EQ(split("hello, world!", 'l', false), v);
	}

	/* SPLIT END */

	/* JOIN */
	TEST(stringops_test, join_empty_vector)
	{
		auto v = std::vector<std::string>{};
		ASSERT_EQ(join(v, ','), "");
	}
	TEST(stringops_test, join_single_vector)
	{
		auto v = std::vector<std::string>{ "hello" };
		ASSERT_EQ(join(v, ','), "hello");
	}
	TEST(stringops_test, join_multi_vector)
	{
		auto v = std::vector<std::string>{ "hello", "world"};
		ASSERT_EQ(join(v, ','), "hello,world");
	}

	/* JOIN END */



	/* LOWERCASE */

	TEST(stringops_test, lowercase_empty_whitespace)
	{
		EXPECT_EQ(lowercase(""), "");
		EXPECT_EQ(lowercase(" \t"), " \t");
	}

	TEST(stringops_test, lowercase_nonempty)
	{
		EXPECT_EQ(lowercase("abc123"), "abc123");
		EXPECT_EQ(lowercase("AbC123"), "abc123");
	}

	/* LOWERCASE END */

	/* UPPERCASE */
	TEST(stringops_test, uppercase_empty_whitespace)
	{
		EXPECT_EQ(uppercase(""), "");
		EXPECT_EQ(uppercase(" \t"), " \t");
	}

	TEST(stringops_test, uppercase_nonempty)
	{
		EXPECT_EQ(uppercase("ABC123"), "ABC123");
		EXPECT_EQ(uppercase("AbC123"), "ABC123");
	}

	/* UPPERCASE END */
}