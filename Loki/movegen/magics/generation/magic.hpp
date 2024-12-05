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
#pragma once
#include "position/bitboard.hpp"

namespace loki::movegen::magics::generation
{
	struct magic_attack
	{
		position::bitboard attack = 0;
		size_t age = 0;
	};

	class magic
	{
	public:
		std::vector<magic_attack> attacks; // Attack table for the given square
		position::bitboard mask;
		position::bitboard magic_number;
		uint64_t shift;

	public:
		magic(size_t n) :
			attacks(n),
			mask{ 0 },
			magic_number{ 0 },
			shift{ 0 }
		{}
		magic() : magic(0)
		{}

		/// <summary>
		/// Given an occupancy bitboard, compute the index of the attack mask.
		/// </summary>
		/// <param name="occupancy">A bitboard with all occupancies</param>
		/// <returns>An index into the attack table</returns>
		constexpr size_t get_index(position::bitboard occupancy) const noexcept
		{
			auto occ = occupancy.get_raw();
			occ &= mask.get_raw();
			occ *= magic_number.get_raw();
			occ >>= 64 - shift;
			return static_cast<size_t>(occ);
		}
	};
}