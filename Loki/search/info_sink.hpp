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

#include <chrono>
#include <memory>
#include <vector>

namespace loki::search
{

	class info_sink
	{
	public:
		virtual ~info_sink() = default;

		virtual void info(
			size_t depth,
			search_score_t score,
			size_t seldepth,
			std::chrono::milliseconds time,
			size_t nodes,
			size_t nps,
			std::vector<movegen::move> pv) = 0;
		virtual void bestmove(movegen::move move) = 0;
	};

	using info_sink_t = std::unique_ptr<info_sink>;

	class null_sink : public info_sink
	{
	public:
		void info(
			size_t depth,
			search_score_t score,
			size_t seldepth,
			std::chrono::milliseconds time,
			size_t nodes,
			size_t nps,
			std::vector<movegen::move> pv) override
		{}

		void bestmove(movegen::move move) override
		{}
	};
}
