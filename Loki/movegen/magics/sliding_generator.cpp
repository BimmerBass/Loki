#include "sliding_generator.hpp"

namespace loki::movegen::magics
{
	using namespace loki::position;

	std::vector<bitboard> sliding_generator::relevant_occupancies(square sq) const
	{
		std::vector<size_t> bit_indices;
		std::vector<bitboard> permutations;
		auto mask = occupancy_mask(sq);
		size_t n = 1ULL << mask.num_one_bits();

		// Get the indices of the set bits
		for (auto s = A1; s <= H8; s++)
		{
			if (mask.is_one_at(s))
				bit_indices.push_back(s);
		}

		// We loop over n subsets, each naturally having different combinations of bits set
		// seeing as they're different numbers.
		// To represent popcount(mask) at most popcount(mask) bits are set in subset
		// which means that subset represents a single permutation of mask
		// then we just mask the i'th subset bit to our mask's bit positions
		for (bitboard_t subset = 0; subset < n; subset++)
		{
			bitboard occupancy = 0;

			for (size_t i = 0; i < mask.num_one_bits(); i++)
			{
				if (subset & (1ULL << i))
					occupancy.set_one_at(bit_indices[i]);
			}
			permutations.push_back(occupancy);
		}
		return permutations;
	}

	bitboard sliding_generator::occupancy_mask(square sq) const
	{
		bitboard_t mask =
			((RANK_MASKS[RANK_1] | RANK_MASKS[RANK_8]) & ~RANK_MASKS[sq.rank()]) |
			((FILE_MASKS[FILE_A] | FILE_MASKS[FILE_H]) & ~FILE_MASKS[sq.file()]);
		return attack(sq, 0ULL) & ~mask;
	}
}