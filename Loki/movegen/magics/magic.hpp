#pragma once
#include "position/bitboard.hpp"

namespace loki::movegen::magics
{
	class magic
	{
	public:
		std::vector<position::bitboard> attacks; // Attack table for the given square
		position::bitboard mask;
		position::bitboard magic_number;
		uint64_t shift;

	public:
		magic(size_t n) :
			attacks(n, 0ULL),
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
		inline constexpr size_t get_index(position::bitboard occupancy) const noexcept
		{
			auto occ = occupancy.get_raw();
			occ &= mask.get_raw();
			occ *= magic_number.get_raw();
			occ >>= 64 - shift;
			return static_cast<size_t>(occ);
		}
	};
}