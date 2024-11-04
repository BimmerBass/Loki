#include "pch.hpp"
#include "loki/position/square.hpp"

using namespace loki::position;

namespace position_tests
{
	class square_testf :
		public ::testing::Test,
		public ::testing::WithParamInterface<std::tuple<e_rank,e_file,e_square, std::string>>
	{};
	INSTANTIATE_TEST_SUITE_P(
		square_tests,
		square_testf,
		::testing::Values(
			std::make_tuple(RANK_1, FILE_A, A1, "A1"),
			std::make_tuple(RANK_4, FILE_E, E4, "e4"),
			std::make_tuple(RANK_8, FILE_H, H8, "H8")));

	TEST_P(square_testf, test_ctor)
	{
		auto [_, unused, sq, unused1] = GetParam();
		square sqq = sq;
		ASSERT_EQ(sqq.value(), sq);
	}
	TEST_P(square_testf, test_ctors)
	{
		auto [r, f, sq, alg] = GetParam();
		square sq1 = sq;
		square sq2(r, f);
		square sq3(alg);

		EXPECT_EQ(sq1.value(), sq);
		EXPECT_EQ(sq2.value(), sq);
		EXPECT_EQ(sq3.value(), sq);
	}
	TEST_P(square_testf, test_file_rank)
	{
		auto [r, f, sq, _] = GetParam();
		square sq1 = sq;
		EXPECT_EQ(sq1.rank(), r);
		EXPECT_EQ(sq1.file(), f);
	}
}