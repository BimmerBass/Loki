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
#include <optional>
#include <string>
#include <vector>

#include "position/game_state.hpp"
#include "search/limits.hpp"

namespace loki::uci
{
	// We want to test that uci_parser correctly calls loki_context's methods so we will mock its interface
	class context_interface
	{
	public:
		virtual ~context_interface() {}

		//
		//	Below functions correspond to the "GUI to engine" bullet-points in 
		//	https://gist.github.com/DOBRO/2592c6dad754ba67e6dcaec8c90165bf
		//
		virtual void uci() const = 0;
		virtual void debug() const = 0;
		virtual void isready() const = 0;
		virtual void setoption(
			std::string name,
			std::optional<std::string> value) = 0;
		virtual void register_() const = 0;
		virtual void ucinewgame() = 0;
		virtual void position(
			const std::string& fen,
			const std::vector<std::string>& moves) = 0;
		virtual void go(
			const search::limits* limits) = 0;
		virtual void stop() = 0;
		virtual void ponderhit() = 0;
	};
}