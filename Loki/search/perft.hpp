// Loki, a UCI-compliant chess playing software
// Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
// Loki is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Loki is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#pragma once
#include <iostream>
#include "position/search_position.hpp"
#include "movegen/magics/magic_index.hpp"

namespace loki::search
{
	class perft
	{
	private:
		std::string m_fen;
		std::ostream& m_os;

		position::search_position_t m_position;
		size_t m_nodes;
		size_t m_nps;

		movegen::magics::magic_index_t m_rook_index, m_bishop_index;
	public:
		perft(std::ostream& os, movegen::magics::magic_index_t rook_index, movegen::magics::magic_index_t bishop_index)
			: m_os{ os }, 
			m_rook_index{ rook_index },
			m_bishop_index{ bishop_index },
			m_fen{}, 
			m_position{ nullptr },
			m_nodes{ 0 }, 
			m_nps{ 0 }
		{}
		
		size_t run(const std::string& fen, size_t depth);
	private:

		void run_internal(size_t depth);
	};
}