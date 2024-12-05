#pragma once
#include "position/bitboard.hpp"
#include "position/square.hpp"

namespace loki::movegen::magics
{
	class sliding_generator
	{
	public:
		/// <summary>
		/// Generate all combinations of occupancies for a given square.
		/// </summary>
		/// <param name="sq">The square</param>
		/// <returns>A vector of possible occupancies</returns>
		std::vector<position::bitboard> relevant_occupancies(position::square sq) const;

		/// <summary>
		/// Generate a base-occupancy mask for the piece (i.e. all ray-attacks until, but excluding, the border files/ranks)
		/// </summary>
		/// <param name="sq">The square</param>
		/// <returns>A bitboard representing the occupancy.</returns>
		position::bitboard occupancy_mask(position::square sq) const;

		/// <summary>
		/// Generate an attack bitboard for a square and a set of occupied squares.
		/// </summary>
		/// <param name="sq">The square for which to generate</param>
		/// <param name="occupancy_mask">A mask for all occupied squares</param>
		/// <returns>A bitboard with 1's where an attack/move is possible.</returns>
		virtual position::bitboard attack(position::square sq, position::bitboard occupancy_mask) const = 0;
	};
}