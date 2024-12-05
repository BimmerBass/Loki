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
#include <vector>
#include "sliding_generator.hpp"
#include "magic.hpp"

namespace loki::movegen::magics::generation
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