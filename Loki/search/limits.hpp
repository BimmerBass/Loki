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
#include <thread>

namespace loki::search
{
	struct limits
	{
		friend class search_thread;
	private:
		mutable std::optional<std::stop_source> stop_source = std::nullopt;
	public:
		
		using clock_t = std::chrono::steady_clock;
		using timepoint_t = clock_t::time_point;

		std::vector<movegen::move> searchmoves{};
		std::atomic_bool pondering = false;

		std::optional<uint64_t> time = std::nullopt;
		std::optional<uint64_t> inc = std::nullopt;
		std::optional<uint64_t> movetime = std::nullopt;
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
		std::chrono::milliseconds time_elapsed(timepoint_t now = clock_t::now()) const noexcept
		{
			auto duration = now - start_time;
			return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
		}

		/// <summary>
		/// Determine whether a configured node or time limit has been reached.
		/// </summary>
		bool should_stop(size_t nodes_searched, timepoint_t now = clock_t::now()) const noexcept
		{
			if (pondering.load(std::memory_order_acquire))
				return false;
			
			if (nodes && nodes_searched >= *nodes)
				return true;

			const bool should_check_time =
				!infinite && !mate && (nodes_searched & 1023) == 0;
			return should_check_time && has_time_limit() && now >= endtime();
		}

		/// <summary>
		/// Check if the time has expired and optionally set the stop token
		/// </summary>
		void check_stopping_conditions(size_t nodes_searched) const noexcept
		{
			if (stop_source.has_value() && should_stop(nodes_searched))
				stop_source.value().request_stop();
		}

		limits();
		limits(const limits& other);
		limits& operator=(const limits& other);
		limits(limits&&) noexcept;
		limits& operator=(limits&&) noexcept;
	private:
		inline bool has_time_limit() const noexcept
		{
			return movetime || time || inc;
		}

		// Use time management heuristics (very simple ATM) to calculate the remaining search time for the given limits.
		timepoint_t endtime() const noexcept
		{
			if (movetime.has_value())
				return start_time + std::chrono::milliseconds(movetime.value());

			auto mtg_estimate = movestogo.value_or(45);
			if (mtg_estimate == 0)
				mtg_estimate = 1;

			auto total_time = (time.value_or(0) / mtg_estimate) + inc.value_or(0);

			assert(total_time >= 0);
			return start_time + std::chrono::milliseconds(total_time);
		}
	};
}
