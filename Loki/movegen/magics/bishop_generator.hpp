#pragma once
#include "magic_generator.hpp"

namespace loki::movegen::magics
{
	class bishop_generator : public sliding_generator
	{
	public:
		position::bitboard attack(position::square sq, position::bitboard occupancy_mask) const;
	};
}