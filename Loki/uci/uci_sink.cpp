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

#include "uci_sink.hpp"

#include <ostream>

template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

using namespace loki;
using namespace loki::uci;

void uci_sink::info(
	size_t depth,
	search_score_t score,
	size_t seldepth,
	std::chrono::milliseconds time,
	size_t nodes,
	size_t nps,
	std::vector<movegen::move> pv)
{
	const auto to_string_v = overloads
	{
		[](const cp_score& cp) { return std::format("cp {}", cp.cp); },
		[](const mate_score& ms) { return std::format("mate {}", ms.in_moves); }
	};

	m_context.out
		<< "info depth " << depth
		<< " score " << std::visit(to_string_v, score)
		<< " seldepth " << seldepth
		<< " time " << time.count()
		<< " nodes " << nodes
		<< " nps " << nps;

	if (!pv.empty())
	{
		m_context.out << " pv";
		for (const auto& move : pv)
			m_context.out << ' ' << move.to_string();
	}

	m_context.out << std::endl;
}

void uci_sink::bestmove(movegen::move move)
{
	m_context.out << "bestmove ";
	if (move == movegen::MOVE_NULL)
		m_context.out << "0000";
	else
		m_context.out << move.to_string();
	m_context.out << std::endl;
}
