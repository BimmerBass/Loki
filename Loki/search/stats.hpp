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
#include "defs.hpp"
#include "pv_table.hpp"

namespace loki::search
{
	/// <summary>
	/// search_statistics holds the statistics gathered during search that will be used for UCI outputting.
	/// </summary>
	struct search_statistics
	{
		pv_table<> pv_table;
		size_t selective_depth;
		size_t nodes;

		void clear() noexcept
		{
			pv_table.clear();
			selective_depth = 0;
			nodes = 0;
		}
	};
}