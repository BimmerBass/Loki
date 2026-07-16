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
#include "search/perft.hpp"

using namespace loki;
using namespace movegen;
using namespace search;

loki_engine::loki_engine()
	:	_position{ std::nullopt }, 
		rook_index{ std::make_shared<magics::hardcoded_index<ROOK>>() },
		bishop_index{ std::make_shared<magics::hardcoded_index<BISHOP>>() },
		_main_thread(0)
{ }

bool loki_engine::set_position(const position::game_state& state, const std::vector<move>& moves)
{
	auto newPos = make_position(state);

	for (const auto& move : moves)
	{
		if (!newPos->make_move(move))
			return false;
	}

	_position = newPos;
	return true;
}

position::search_position_t loki_engine::make_position(const position::game_state& state) const
{
	return position::make(
		std::make_shared<position::game_state>(state),
		bishop_index, rook_index);
}

void loki_engine::set_position(const position::game_state& state)
{
	_position = position::make(
		std::make_shared<position::game_state>(state),
		bishop_index, rook_index);
}

void loki_engine::set_position(position::search_position_t state)
{
	_position = std::move(state);
}

void loki_engine::clear()
{
	_position = std::nullopt;
}


void loki_engine::search(
	const limits limits,
	search_thread::callback_t finished_callback,
	info_sink_t sink)
{
	if (!_position.has_value())
		throw_msg<loki_exception>("cannot search without a position object");
	_main_thread.search(_position.value(), limits, finished_callback, std::move(sink));
}


[[maybe_unused]]
size_t loki_engine::perft(size_t depth, std::ostream& out) const
{
	search::perft runner(out, rook_index, bishop_index);
	const auto& fen = position()->to_fen();

	return runner.run(fen, depth);
}
