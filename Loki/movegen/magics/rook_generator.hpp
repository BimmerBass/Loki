#pragma once
#include "sliding_generator.hpp"

namespace loki::movegen::magics
{
	class rook_generator : public sliding_generator
	{
		using bitboard = position::bitboard;
		using square = position::square;
	public:
		bitboard attack(square sq, bitboard occupancy_mask) const;
	};
}