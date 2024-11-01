#include "pch.hpp"
#include "Loki/position/castle_rights.hpp"


namespace position_tests
{
	using namespace loki;
	// parameterized testing: https://www.sandordargo.com/blog/2019/04/24/parameterized-testing-with-gtest
	class castle_rights_test :
		public ::testing::Test,
		public ::testing::WithParamInterface<std::tuple<side, castling_direction>>
	{
	protected:
		castle_rights_test() : rights() {}

		loki::position::castle_rights rights;
	};

	// hacky wrapper functions to allow for runtime test cases
	bool runtime_can_castle(const loki::position::castle_rights& crs, side s, castling_direction d) noexcept
	{
		return (s == WHITE) ?
			((d == KINGSIDE) ? crs.can_castle<WHITE, KINGSIDE>() : crs.can_castle<WHITE, QUEENSIDE>()) :
			((d == KINGSIDE) ? crs.can_castle<BLACK, KINGSIDE>() : crs.can_castle<BLACK, QUEENSIDE>());
	}
	void runtime_set(position::castle_rights& crs, side s, castling_direction d, bool value) noexcept
	{
		if (s == WHITE)
		{
			if (d == KINGSIDE) crs.set<WHITE, KINGSIDE>(value);
			else crs.set<WHITE, QUEENSIDE>(value);
		}
		else
		{
			if (d == KINGSIDE) crs.set<BLACK, KINGSIDE>(value);
			else crs.set<BLACK, QUEENSIDE>(value);
		}
	}

	TEST_F(castle_rights_test, initially_false)
	{
		EXPECT_EQ((rights.can_castle<WHITE, KINGSIDE>()), false);
		EXPECT_EQ((rights.can_castle<WHITE, QUEENSIDE>()), false);
		EXPECT_EQ((rights.can_castle<BLACK, KINGSIDE>()), false);
		EXPECT_EQ((rights.can_castle<BLACK, QUEENSIDE>()), false);
	}

	TEST_P(castle_rights_test, set_false_to_true)
	{
		// all are initially_false (ref. above test)
		// check that setting one doesn't set anything else
		auto [side, direction] = GetParam();

		ASSERT_FALSE(runtime_can_castle(rights, side, direction));
		runtime_set(rights, side, direction, true);
		ASSERT_TRUE(runtime_can_castle(rights, side, direction));
	}
	INSTANTIATE_TEST_SUITE_P(
		castle_rights_test,
		castle_rights_test,
		::testing::Values(
			std::make_tuple(WHITE, KINGSIDE),
			std::make_tuple(WHITE, QUEENSIDE),
			std::make_tuple(BLACK, KINGSIDE),
			std::make_tuple(BLACK, QUEENSIDE)
		)
	);
	TEST_P(castle_rights_test, set_true_to_false)
	{
		// all are initially_false (ref. above test)
		// check that setting one doesn't set anything else
		auto [side, direction] = GetParam();

		runtime_set(rights, side, direction, true);
		ASSERT_TRUE(runtime_can_castle(rights, side, direction));
		runtime_set(rights, side, direction, false);
		ASSERT_FALSE(runtime_can_castle(rights, side, direction));
	}
	TEST_P(castle_rights_test, set_false_to_false)
	{
		auto [side, direction] = GetParam();

		// pre-cond.
		ASSERT_FALSE(runtime_can_castle(rights, side, direction));
		runtime_set(rights, side, direction, false);
		ASSERT_FALSE(runtime_can_castle(rights, side, direction));
	}
	TEST_P(castle_rights_test, set_true_to_true)
	{
		auto [side, direction] = GetParam();

		// pre-cond.
		runtime_set(rights, side, direction, true);
		ASSERT_TRUE(runtime_can_castle(rights, side, direction));
		runtime_set(rights, side, direction, true);
		ASSERT_TRUE(runtime_can_castle(rights, side, direction));
	}

	TEST_F(castle_rights_test, set_single_value)
	{
		// assumption: initially_empty is passing
		rights.set<WHITE, KINGSIDE>(true);

		// we know from set_false_to_true that white kingside is true now.
		// check that nothing else has been changed.
		EXPECT_FALSE((rights.can_castle<WHITE, QUEENSIDE>()));
		EXPECT_FALSE((rights.can_castle<BLACK, KINGSIDE>()));
		EXPECT_FALSE((rights.can_castle<BLACK, QUEENSIDE>()));
	}
}