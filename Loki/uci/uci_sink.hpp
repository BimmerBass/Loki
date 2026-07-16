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

#include "search/info_sink.hpp"
#include "uci/context.hpp"

namespace loki::uci
{
	class uci_sink final : public search::info_sink
	{
	public:
		explicit uci_sink(context& ctx) noexcept : m_context{ ctx }
		{}

		void info(
			size_t depth,
			search_score_t score,
			size_t seldepth,
			std::chrono::milliseconds time,
			size_t nodes,
			size_t nps,
			std::vector<movegen::move> pv) override;

		void bestmove(movegen::move move) override;

	private:
		context& m_context;
	};
}
