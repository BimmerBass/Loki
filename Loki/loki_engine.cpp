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

#include "loki_engine.hpp"
#include "movegen/magics/hardcoded_index.hpp"
#include "position/search_position.hpp"

using namespace loki;
using namespace movegen;

loki_engine::loki_engine()
	:	game_state{ std::nullopt }, 
		rook_index{ std::make_shared<magics::hardcoded_index<ROOK>>() },
		bishop_index{ std::make_shared<magics::hardcoded_index<BISHOP>>() }
{ }


bool loki_engine::set_position(const position::game_state& state, const std::vector<move>& moves)
{
	auto pos = position::make(
		std::make_shared<position::game_state>(state),
		bishop_index, rook_index);

	for (const auto& move : moves)
	{
		if (!pos->make_move(move))
			return false;
	}

	game_state = *pos->state();
	return true;
}