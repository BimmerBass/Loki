#include "pch.hpp"
#include "Loki/movegen/magics/generation/bishop_generator.hpp"
#include "Loki/movegen/magics/generation/sliding_generator.cpp"

namespace movegen_tests::generation_tests::magics_tests
{
	using namespace loki::position;
	using namespace loki::movegen::magics;
	using namespace loki::movegen::magics::generation;

	TEST(sliding_generator_tests, test_occupancy_permutations)
	{
		bishop_generator g;
		
		for (auto sq = A1; sq <= H8; sq++)
		{
			auto base_mask = g.occupancy_mask(sq);
			auto occupancies = g.relevant_occupancies(sq);

			// Check for the correct number of permutations
			ASSERT_EQ(occupancies.size(), 1ULL << base_mask.num_one_bits());

			// Check that each permutation is valid
			for (auto& p : occupancies)
			{
				ASSERT_EQ(p.get_raw() & ~base_mask.get_raw(), 0);
			}
			
			// Check that all values are unique
			std::sort(occupancies.begin(), occupancies.end());
			ASSERT_EQ(
				std::unique(occupancies.begin(), occupancies.end()),
				occupancies.end());
		}
	}
}
