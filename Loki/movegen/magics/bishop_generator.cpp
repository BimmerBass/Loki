//
//	Loki, a UCI-compliant chess playing software
//	Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
//	Loki is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	Loki is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
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