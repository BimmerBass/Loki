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
#include "rook_generator.hpp"

namespace loki::movegen::magics::generation
{
	using namespace loki::position;

	bitboard_t rook_generator::attack(square sq, bitboard_t occupancy_mask) const
	{
		using namespace loki::position;
		bitboard_t result = 0;

		for (auto r = sq.rank() + 1; r <= RANK_8; r++) // up
		{
			auto cur_sq = sq.file() + r * 8;
			result = set_one_at(result, cur_sq);
			if (is_one_at(occupancy_mask, cur_sq))
				break;
		}
		for (auto r = sq.rank() - 1; r >= RANK_1; r--) // down
		{
			auto cur_sq = sq.file() + r * 8;
			result = set_one_at(result, cur_sq);
			if (is_one_at(occupancy_mask, cur_sq))
				break;
		}
		for (auto f = sq.file() + 1; f <= FILE_H; f++) // right
		{
			auto cur_sq = f + sq.rank() * 8;
			result = set_one_at(result, cur_sq);
			if (is_one_at(occupancy_mask, cur_sq))
				break;
		}
		for (auto f = sq.file() - 1; f >= FILE_A; f--) // left
		{
			auto cur_sq = f + sq.rank() * 8;
			result = set_one_at(result, cur_sq);
			if (is_one_at(occupancy_mask, cur_sq))
				break;
		}
		return result;
	}
}