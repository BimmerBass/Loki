#include "pch.hpp"
#include "Loki/movegen/magics/generation/rook_generator.hpp"
#include "Loki/movegen/magics/generation/rook_generator.cpp"

namespace movegen_tests::generation_tests::magics_tests
{
	using namespace loki::position;
	using namespace loki::movegen::magics;
	using namespace loki::movegen::magics::generation;

	class rook_test_attack :
		public ::testing::Test,
		public ::testing::WithParamInterface<std::tuple<square, bitboard_t, bitboard_t>>
	{
	public:
		rook_test_attack()
			: gen{}
		{
		}

	protected:
		rook_generator gen;
	};
	INSTANTIATE_TEST_SUITE_P(
		magics_tests,
		rook_test_attack,
		::testing::Values(
			std::make_tuple(A1, 0x01010101010101fe, 0x0), // corner, no occupancies
			std::make_tuple(H5, 0x8080807f80808080, 0x0), // edge, no occupancies
			std::make_tuple(E4, 0x10101010ef101010, 0x0), // central, no occupancies
			std::make_tuple(E5, 0x101010ec10101010, 0x0000000400000000), // one occupancy
			std::make_tuple(C4, 0x000000041b040404, 0x0000000411000000), // multiple occupancies
			std::make_tuple(F6, 0x2020df2020202020, 0x0000000200040000) // irrelevant occupancy
		));

	TEST_P(rook_test_attack, attack_empty_test)
	{
		auto [sq, expected, occ] = GetParam();
		auto generated = gen.attack(sq, occ);
		ASSERT_EQ(generated.get_raw(), expected);
	}

	TEST_P(rook_test_attack, test_masks)
	{
		for (square s = A1; s <= H8; s++)
		{
			bitboard_t border = 0ULL;
			if (s.rank() != RANK_1)
				border |= RANK_MASKS[RANK_1];
			if (s.rank() != RANK_8)
				border |= RANK_MASKS[RANK_8];
			if (s.file() != FILE_A)
				border |= FILE_MASKS[FILE_A];
			if (s.file() != FILE_H)
				border |= FILE_MASKS[FILE_H];
			auto mask = gen.occupancy_mask(s);
			auto attack = gen.attack(s, 0ULL);
			ASSERT_EQ(mask.get_raw(), attack & ~border);
		}
	}
}