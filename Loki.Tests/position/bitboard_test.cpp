#include "pch.hpp"
#include "loki/position/bitboard.hpp"
#include "loki/position/bitboard.cpp"

using namespace loki::position;

namespace position_tests
{
#pragma region IS_ONE_AT
	TEST(bitboard_test, is_one_at_zero)
	{
		ASSERT_FALSE(is_one_at(0x1ULL, 1));
	}
	TEST(bitboard_test, is_one_at_one)
	{
		ASSERT_TRUE(is_one_at(0x1ULL, 0));
	}
#pragma endregion
#pragma region POPCOUNT
	class popcount_test :
		public ::testing::Test,
		public ::testing::WithParamInterface<std::tuple<bitboard_t, size_t>>
	{};
	INSTANTIATE_TEST_SUITE_P(
		bitboard_test,
		popcount_test,
		::testing::Values(
			std::make_tuple(0, 0),
			std::make_tuple(~0ULL, 64),
			std::make_tuple(0x5555555555555555ULL, 32) // every other is 1
		));

	TEST_P(popcount_test, popcount)
	{
		auto [xi, ni] = GetParam();
		ASSERT_EQ(popcount(xi), ni);
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
		ASSERT_EQ(scan_lsb(value), ls1b);
	}
#pragma endregion
#pragma region SCAN_MSB
	TEST_P(significant_bits_test, test_scan_msb)
	{
		auto [value, _, ms1b] = GetParam();
		ASSERT_EQ(scan_msb(value), ms1b);
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
		ASSERT_EQ(set_one_at(0, i), bitboard_t(1) << i);
	}
	TEST_P(set_one_at_test, set_one_at_already_one)
	{
		auto i = GetParam();
		auto initial = 1ULL << i;
		ASSERT_EQ(set_one_at(initial, i), initial);
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
		bitboard_t x = 1ULL << i;
		ASSERT_EQ(toggle_at(x, i), 0ULL);
	}
	TEST_P(toggle_at_test, toggle_zero_bit)
	{
		auto i = GetParam();
		ASSERT_EQ(toggle_at(0ULL, i), 1ULL << i);
	}
#pragma endregion
#pragma region POP_LSB
	TEST(bitboard_test, pop_lsb_zero)
	{
		ASSERT_EQ(pop_lsb(0), std::nullopt);
	}
	TEST(bitboard_test, pop_lsb_nonzero)
	{
		auto x =
			(bitboard_t(1) << 32) |
			(bitboard_t(1) << 63);
		ASSERT_EQ(pop_lsb(x), std::optional(32ULL));
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
		bitboard_t bb = b;
		bitboard_t shifted;

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
		ASSERT_EQ(shifted, e);
	}
#pragma endregion
}
