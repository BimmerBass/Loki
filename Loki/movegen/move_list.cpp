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

#include "move_list.hpp"
#include "position/square.hpp"
#include <ranges>
#include <algorithm>
#include <array>

namespace loki::movegen
{
	using namespace position;

	std::optional<move> move_list::find(const std::string& move_string) const
	{
		if (move_string.length() < 4 || move_string.length() > 5)
			return std::nullopt;
		square from_sq = move_string.substr(0, 2);
		square to_sq = move_string.substr(2, 2);
		auto type = move_string.length() == 5 ? PROMOTION : NORMAL;
		auto piece = NO_PIECE;
		if (type == PROMOTION)
		{
			switch (std::tolower(move_string[4]))
			{
			case 'n': piece = KNIGHT; break;
			case 'b': piece = BISHOP; break;
			case 'r': piece = ROOK; break;
			case 'q': piece = QUEEN; break;
			default:
				return std::nullopt;
			}
		}

		auto pred = [&](const move& m)
			{
				if (m.from() != from_sq.value() || m.to() != to_sq.value())
					return false;
				if (type == PROMOTION)
					return m.type() == PROMOTION
						&& m.promotion_piece() == piece;
				return true;
			};
		auto it = std::ranges::find_if(m_collection, pred);
		if (it == m_collection.end())
			return std::nullopt;
		return *it;
	}
}