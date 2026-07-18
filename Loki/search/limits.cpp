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

#include "limits.hpp"

#include <utility>

using namespace loki::search;


limits::limits() = default;

limits::limits(const limits& other)
{
	stop_source = other.stop_source;
	searchmoves = other.searchmoves;
	pondering = other.pondering.load(std::memory_order_relaxed);
	time = other.time;
	inc = other.inc;
	movetime = other.movetime;
	infinite = other.infinite;
	movestogo = other.movestogo;
	depth = other.depth;
	nodes = other.nodes;
	mate = other.mate;
	start_time = other.start_time;
}

limits& limits::operator=(const limits& other)
{
	if (this == &other)
		return *this;

	stop_source = other.stop_source;
	searchmoves = other.searchmoves;
	pondering = other.pondering.load(std::memory_order_relaxed);
	time = other.time;
	inc = other.inc;
	movetime = other.movetime;
	infinite = other.infinite;
	movestogo = other.movestogo;
	depth = other.depth;
	nodes = other.nodes;
	mate = other.mate;
	start_time = other.start_time;
	return *this;
}

limits::limits(limits&& other) noexcept :
	stop_source{std::move(other.stop_source)},
	searchmoves{std::move(other.searchmoves)},
	pondering{other.pondering.load(std::memory_order_relaxed)},
	time{std::move(other.time)},
	inc{std::move(other.inc)},
	movetime{std::move(other.movetime)},
	infinite{other.infinite},
	movestogo{std::move(other.movestogo)},
	depth{std::move(other.depth)},
	nodes{std::move(other.nodes)},
	mate{std::move(other.mate)},
	start_time{other.start_time}
{}

limits& limits::operator=(limits&& other) noexcept
{
	if (this == &other)
		return *this;

	stop_source = std::move(other.stop_source);
	searchmoves = std::move(other.searchmoves);
	pondering.store(other.pondering.load(std::memory_order_relaxed), std::memory_order_relaxed);
	time = std::move(other.time);
	inc = std::move(other.inc);
	movetime = std::move(other.movetime);
	infinite = other.infinite;
	movestogo = std::move(other.movestogo);
	depth = std::move(other.depth);
	nodes = std::move(other.nodes);
	mate = std::move(other.mate);
	start_time = other.start_time;
	return *this;
}
