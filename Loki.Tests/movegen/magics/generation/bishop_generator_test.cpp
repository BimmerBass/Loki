#include "pch.hpp"
#include "Loki/movegen/magics/generation/bishop_generator.hpp"
#include "Loki/movegen/magics/generation/bishop_generator.cpp"

namespace movegen_tests::generation_tests::magics_tests
{
	using namespace loki::position;
	using namespace loki::movegen::magics;
	using namespace loki::movegen::magics::generation;

	class bishop_test_attack :
		public ::testing::Test,
		public ::testing::WithParamInterface<std::tuple<square, bitboard_t, bitboard_t>>
	{
	public:
		bishop_test_attack()
			: gen{}
		{}

	protected:
		bishop_generator gen;
	};
	INSTANTIATE_TEST_SUITE_P(
		magics_tests,
		bishop_test_attack,
		::testing::Values(
			std::make_tuple(A1, 0x8040201008040200, 0x0), // corner, no occupancies
			std::make_tuple(H5, 0x1020400040201008, 0x0), // edge, no occupancies
			std::make_tuple(E5, 0x8244280028448201, 0x0), // central, no occupancies
			std::make_tuple(E4, 0x0182442800284080, 0x0000000000080000), // one occupancy
			std::make_tuple(C4, 0x0000010a000a1100, 0x0000010804001000), // multiple occupancies
			std::make_tuple(F6, 0x8850005088040201, 0x0080220000000000) // irrelevant occupancy
		));

	TEST_P(bishop_test_attack, attack_empty_test)
	{
		auto [sq, expected, occ] = GetParam();
		auto generated = gen.attack(sq, occ);
		ASSERT_EQ(generated.get_raw(), expected);
	}

	TEST_P(bishop_test_attack, test_masks)
	{
		auto border_mask = (FILE_MASKS[FILE_A] | FILE_MASKS[FILE_H] | RANK_MASKS[RANK_1] | RANK_MASKS[RANK_8]);
		for (auto s = A1; s <= H8; s++)
		{
			auto mask = gen.occupancy_mask(s);
			auto attack = gen.attack(s, 0ULL);
			ASSERT_EQ(mask.get_raw(), attack & ~border_mask);
		}
	}
}