#include "pch.hpp"
#include "Loki/movegen/move_list.hpp"

using namespace loki::movegen;
using namespace loki;

namespace move_tests
{
	TEST(move_list_tests, test_push_back)
	{
		move_list ml;

		for (size_t i = 0; i < ml.max_size; i++)
		{
			ASSERT_EQ(ml.size(), i);
			ml.push_back(move_t(i + 1));

			ASSERT_EQ(ml[i], i + 1);
			if (i > 0)
				ASSERT_EQ(ml[i - 1], i);
			if (i < ml.max_size - 1)
				ASSERT_THROW(ml[i + 1], move_list::move_list_error);
		}
		ASSERT_EQ(ml.size(), ml.max_size);
		ASSERT_THROW(ml.push_back(0), move_list::move_list_error);
	}

	TEST(move_list_tests, test_clear)
	{
		move_list ml;
		ml.push_back(0);
		ml.push_back(0);

		ASSERT_EQ(ml.size(), 2);
		ml.clear();
		ASSERT_EQ(ml.size(), 0);
	}
	TEST(move_list_tests, test_iterators)
	{
		move_list ml;
		for (size_t i = 0; i < ml.max_size / 2; i++)
		{
			ml.push_back(move_t(i+1));
		}
		size_t j = 0;

		for (const auto& m : ml)
		{
			ASSERT_EQ(m, j + 1);
			j++;
		}
	}
}