#include "pch.hpp"
#include "Loki/movegen/magics/magic_generator.hpp"
#include "Loki/movegen/magics/magic_generator.cpp"

#include "Loki/movegen/magics/rook_generator.hpp"
#include "Loki/movegen/magics/bishop_generator.hpp"

namespace movegen_tests::magics_tests
{
	using namespace loki::position;
	using namespace loki::movegen::magics;

	class test_with_generator :
		public ::testing::Test,
		public ::testing::WithParamInterface<std::shared_ptr<sliding_generator>>
	{
	};
	INSTANTIATE_TEST_SUITE_P(
		magics_tests,
		test_with_generator,
		::testing::Values(
			std::make_shared<rook_generator>(),
			std::make_shared<bishop_generator>()
		));

	TEST_P(test_with_generator, test_magics)
	{
		auto ptr = GetParam();
		magic_generator generator(ptr);
		auto magics = generator.generate();

		for (auto sq = A1; sq <= H8; sq++)
		{
			auto magic = magics[sq];
			auto occupancies = ptr->relevant_occupancies(sq);

			// Check enough attacks
			ASSERT_EQ(magic.attacks.size(), 1ULL << magic.shift);

			// Go through each occupancy and check that we get an index holding the correct attack.
			for (auto& occ : occupancies)
			{
				auto inx = magic.get_index(occ);
				auto attack = ptr->attack(sq, occ);
				ASSERT_TRUE(inx >= 0 && inx <= magic.attacks.size());

				auto x = magic.attacks[inx].attack.get_raw();
				auto y = attack.get_raw();
				ASSERT_EQ(magic.attacks[inx].attack, attack);
			}
		}
	}
}