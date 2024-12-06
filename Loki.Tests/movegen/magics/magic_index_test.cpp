#include "pch.hpp"
#include "Loki/movegen/magics/magic_index.hpp"
#include "Loki/movegen/magics/hardcoded_index.hpp"
#include "Loki/movegen/magics/magic_index.cpp"
#include "Loki/movegen/magics/hardcoded_index.cpp"

#include "Loki/movegen/magics/generation/rook_generator.hpp"
#include "Loki/movegen/magics/generation/bishop_generator.hpp"

namespace movegen_tests::magics_tests
{
	using namespace loki;
	using namespace loki::position;
	using namespace loki::movegen::magics;
	using namespace loki::movegen::magics::generation;

	class test_magic_index :
		public ::testing::Test,
		public ::testing::WithParamInterface<piece>
	{};
	INSTANTIATE_TEST_SUITE_P(
		magics_tests,
		test_magic_index,
		::testing::Values(ROOK, BISHOP));

	TEST_P(test_magic_index, test_hardcoded)
	{
		auto t = GetParam();
		hardcoded_index inx(t);
		sliding_generator* gen;
		switch (t)
		{
		case ROOK:
			gen = new rook_generator();
			break;
		case BISHOP:
			gen = new bishop_generator();
			break;
		default:
			ASSERT_TRUE(false);
			return;
		}


		for (auto sq = A1; sq <= H8; sq++)
		{
			auto occs = gen->relevant_occupancies(sq);

			for (auto occ : occs)
			{
				ASSERT_EQ(inx.attacks(sq, occ), gen->attack(sq, occ));
			}
		}

		delete gen;
	}
}