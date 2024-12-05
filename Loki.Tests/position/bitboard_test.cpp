#include "pch.hpp"
#include "loki/position/bitboard.hpp"
#include "loki/position/bitboard.cpp"

using namespace loki::position;

namespace position_tests
{
#pragma region IS_ONE_AT
	TEST(bitboard_test, is_one_at_zero)
	{
		bitboard x = 0x1;
		ASSERT_FALSE(x.is_one_at(1));
	}
	TEST(bitboard_test, is_one_at_one)
	{
		bitboard x = 0x1;
		ASSERT_TRUE(x.is_one_at(0));
	}
#pragma endregion
#pragma region NUM_ONE_BITS
	class num_one_bits_test :
		public ::testing::Test,
		public ::testing::WithParamInterface<std::tuple<bitboard_t, size_t>>
	{};
	INSTANTIATE_TEST_SUITE_P(
		bitboard_test,
		num_one_bits_test,
		::testing::Values(
			std::make_tuple(0, 0),
			std::make_tuple(~bitboard_t(0), 64),
			std::make_tuple(bitboard_t(0x5555555555555555), 32) // every other is 1
		));

	TEST_P(num_one_bits_test, get_raw)
	{
		auto [xi, ni] = GetParam();
		bitboard x = xi;
		ASSERT_EQ(x.get_raw(), xi);
	}

	TEST_P(num_one_bits_test, num_one_bits)
	{
		auto [xi, ni] = GetParam();
		bitboard x = (bitboard_t)xi;
		ASSERT_EQ(x.num_one_bits(), ni);
	}
#pragma endregion
#pragma region SCAN_LSB
	class significant_bits_test :
		public ::testing::Test,
		public ::testing::WithParamInterface<std::tuple<bitboard_t, std::optional<size_t>, std::optional<size_t>>>
	{};
	INSTANTIATE_TEST_SUITE_P(
		bitboard_test,
		significant_bits_test,
		::testing::Values(
			std::make_tuple(0, std::nullopt, std::nullopt),
			std::make_tuple(0x000f000, 12, 15),
			std::make_tuple(0x8000000000000001, 0, 63)
		));

	TEST_P(significant_bits_test, test_scan_lsb)
	{
		auto [value, ls1b, _] = GetParam();
		bitboard x = value;
		ASSERT_EQ(x.scan_lsb(), ls1b);
	}
#pragma endregion
#pragma region SCAN_MSB
	TEST_P(significant_bits_test, test_scan_msb)
	{
		auto [value, _, ms1b] = GetParam();
		bitboard x = value;
		ASSERT_EQ(x.scan_msb(), ms1b);
	}
#pragma endregion
#pragma region SET_ONE_AT
	class set_one_at_test :
		public ::testing::Test,
		public ::testing::WithParamInterface<size_t>
	{};
	INSTANTIATE_TEST_SUITE_P(
		bitboard_test,
		set_one_at_test,
		::testing::Values(0, 63, 32));

	TEST_P(set_one_at_test, set_one_at_zero)
	{
		auto i = GetParam();
		bitboard x = 0;
		ASSERT_EQ(x.set_one_at(i), bitboard_t(1) << i);
	}
	TEST_P(set_one_at_test, set_one_at_already_one)
	{
		auto i = GetParam();
		auto initial = (bitboard_t)1 << i;
		bitboard x = initial;
		ASSERT_EQ(x.set_one_at(i), initial);
	}
#pragma endregion
#pragma region TOGGLE_AT
	class toggle_at_test : public set_one_at_test {};
	INSTANTIATE_TEST_SUITE_P(
		bitboard_test,
		toggle_at_test,
		::testing::Values(0, 63, 32));

	TEST_P(toggle_at_test, toggle_one_bit)
	{
		auto i = GetParam();
		bitboard x = bitboard_t(1) << i;
		ASSERT_EQ(x.toggle_at(i), 0);
	}
	TEST_P(toggle_at_test, toggle_zero_bit)
	{
		auto i = GetParam();
		bitboard x = 0;
		ASSERT_EQ(x.toggle_at(i), bitboard_t(1) << i);
	}
#pragma endregion
#pragma region POP_LSB
	TEST(bitboard_test, pop_lsb_zero)
	{
		bitboard x = 0;
		ASSERT_EQ(x.pop_lsb(), std::nullopt);
	}
	TEST(bitboard_test, pop_lsb_nonzero)
	{
		bitboard x =
			(bitboard_t(1) << 32) |
			(bitboard_t(1) << 63);
		ASSERT_EQ(x.pop_lsb(), std::optional(32ULL));
	}
#pragma endregion
#pragma region OPERATORS
	class shift_operators_test :
		public ::testing::Test,
		public ::testing::WithParamInterface<std::tuple<bitboard_t, bitboard_t, e_direction>>
	{};
	INSTANTIATE_TEST_SUITE_P(
		bitboard_test,
		shift_operators_test,
		::testing::Values(
			std::make_tuple(0xFF00000000000000, 0, UP),
			std::make_tuple(0xFF00000000000000, 0x00FF000000000000, DOWN),
			std::make_tuple(0x00000000000000FF, 0x000000000000FF00, UP),
			std::make_tuple(0x00000000000000FF, 0x0000000000000000, DOWN),
			std::make_tuple(0x00000000FF000000, 0x000000FF00000000, UP),
			std::make_tuple(0x00000000FF000000, 0x0000000000FF0000, DOWN),
			std::make_tuple(0x0101010101010101, 0x0000000000000000, LEFT),
			std::make_tuple(0x0101010101010101, 0x202020202020202, RIGHT),
			std::make_tuple(0x8080808080808080, 0x4040404040404040, LEFT),
			std::make_tuple(0x8080808080808080, 0x0000000000000000, RIGHT),
			std::make_tuple(0x0808080808080808, 0x0404040404040404, LEFT),
			std::make_tuple(0x0808080808080808, 0x1010101010101010, RIGHT)
		));

	TEST_P(shift_operators_test, test_shift)
	{
		auto [b, e, d] = GetParam();
		bitboard bb = b;
		bitboard shifted;

		switch (d)
		{
		case UP:
			shifted = shift<UP>(bb);
			break;
		case DOWN:
			shifted = shift<DOWN>(bb);
			break;
		case LEFT:
			shifted = shift<LEFT>(bb);
			break;
		case RIGHT:
			shifted = shift<RIGHT>(bb);
			break;
		}
		ASSERT_EQ(shifted, bitboard(e));
	}
	TEST(bitboard_test, equal)
	{
		bitboard_t value = 0x1234ff;
		bitboard x1 = value;
		bitboard x2 = value;

		EXPECT_EQ(x1, value);
		EXPECT_EQ(x2, value);
		EXPECT_EQ(x1, x2);
	}
	TEST(bitboard_test, not_equal)
	{
		bitboard_t v1 = 0x1234ff;
		bitboard_t v2 = 0x1234f;
		bitboard x1 = v1;
		bitboard x2 = v2;

		EXPECT_NE(x1, v2);
		EXPECT_NE(x2, v1);
		EXPECT_NE(x1, x2);
	}
#pragma endregion
}
