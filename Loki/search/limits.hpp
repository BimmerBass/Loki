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
#include "movegen/move.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <tuple>
#include <vector>
#include <chrono>

namespace loki::search
{
	struct limits
	{
		using clock_t = std::chrono::steady_clock;
		using timepoint_t = clock_t::time_point;
		using playtime_t = std::tuple<uint64_t, uint64_t>; // time, increment

		std::vector<movegen::move> searchmoves{};
		bool pondering = false;

		std::optional<playtime_t> wtime = std::nullopt;
		std::optional<playtime_t> btime = std::nullopt;
		std::optional<playtime_t> movetime = std::nullopt;
		bool infinite = false;

		std::optional<size_t> movestogo = std::nullopt;
		std::optional<size_t> depth = std::nullopt;
		std::optional<size_t> nodes = std::nullopt;
		std::optional<size_t> mate = std::nullopt;

		timepoint_t start_time = clock_t::now();

		/// <summary> 
		/// Calculate the time elapsed since the search startes in milliseconds.
		/// </summary>
		/// <returns>A std::chrono::milliseconds duration.</returns>
		std::chrono::milliseconds time_elapsed() const noexcept
		{
			auto duration = clock_t::now() - start_time;
			return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
		}
	};
}
