#pragma once
#include <vector>
#include "sliding_generator.hpp"
#include "magic.hpp"

namespace loki::movegen::magics
{
	class magic_generator
	{
	private:
		std::shared_ptr<sliding_generator> m_generator;
	public:
		magic_generator(const std::shared_ptr<sliding_generator> gen)
			: m_generator{ gen }
		{}

		std::array<magic, position::NUM_SQUARES> generate();
	private:
		magic find_magic(position::square sq);
		position::bitboard generate_valid_random(const position::bitboard& mask) const;
	};
}