#include "bishop_generator.hpp"

namespace loki::movegen::magics
{
	using namespace loki::position;

	bitboard bishop_generator::attack(square sq, bitboard occupancy_mask) const
	{
		bitboard result;
		e_rank r;
		e_file f;

		// up, right
		for (r = sq.rank() + 1, f = sq.file() + 1; r <= RANK_8 && f <= FILE_H; r++, f++)
		{
			auto cur_sq = square(r, f);
			result.set_one_at(cur_sq.value());
			if (occupancy_mask.is_one_at(cur_sq.value()))
				break;
		}
		// up, left
		for (r = sq.rank() + 1, f = sq.file() - 1; r <= RANK_8 && f >= FILE_A; r++, f--)
		{
			auto cur_sq = square(r, f);
			result.set_one_at(cur_sq.value());
			if (occupancy_mask.is_one_at(cur_sq.value()))
				break;
		}
		// down, right
		for (r = sq.rank() - 1, f = sq.file() + 1; r >= RANK_1 && f <= FILE_H; r--, f++)
		{
			auto cur_sq = square(r, f);
			result.set_one_at(cur_sq.value());
			if (occupancy_mask.is_one_at(cur_sq.value()))
				break;
		}
		// down, left
		for (r = sq.rank() - 1, f = sq.file() - 1; r >= RANK_1 && f >= FILE_A; r--, f--)
		{
			auto cur_sq = square(r, f);
			result.set_one_at(cur_sq.value());
			if (occupancy_mask.is_one_at(cur_sq.value()))
				break;
		}
		return result;
	}
}