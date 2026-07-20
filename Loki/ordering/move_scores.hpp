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
#include "movegen/move.hpp"

namespace loki::ordering
{
	static constexpr movegen::move_score_t HISTORY_MAX = 8000; // history \in [-8000;8000]
	static constexpr movegen::move_score_t KILLER_TWO = HISTORY_MAX + 1;
	static constexpr movegen::move_score_t KILLER_ONE = KILLER_TWO + 1;
	static constexpr movegen::move_score_t GOOD_CAPTURE = KILLER_ONE + 1;

	static_assert(-HISTORY_MAX > -0x4000 && GOOD_CAPTURE <= 0x7FFF);
}