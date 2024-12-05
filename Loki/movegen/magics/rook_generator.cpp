#include "rook_generator.hpp"

namespace loki::movegen::magics
{
	using namespace loki::position;

	bitboard rook_generator::attack(square sq, bitboard occupancy_mask) const
	{
		using namespace loki::position;
		bitboard result = 0;

		for (auto r = sq.rank() + 1; r <= RANK_8; r++) // up
		{
			auto cur_sq = sq.file() + r * 8;
			result.set_one_at(cur_sq);
			if (occupancy_mask.is_one_at(cur_sq))
				break;
		}
		for (auto r = sq.rank() - 1; r >= RANK_1; r--) // down
		{
			auto cur_sq = sq.file() + r * 8;
			result.set_one_at(cur_sq);
			if (occupancy_mask.is_one_at(cur_sq))
				break;
		}
		for (auto f = sq.file() + 1; f <= FILE_H; f++) // right
		{
			auto cur_sq = f + sq.rank() * 8;
			result.set_one_at(cur_sq);
			if (occupancy_mask.is_one_at(cur_sq))
				break;
		}
		for (auto f = sq.file() - 1; f >= FILE_A; f--) // left
		{
			auto cur_sq = f + sq.rank() * 8;
			result.set_one_at(cur_sq);
			if (occupancy_mask.is_one_at(cur_sq))
				break;
		}
		return result;
	}
}