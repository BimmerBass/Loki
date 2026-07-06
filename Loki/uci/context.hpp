#pragma once
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
#include <iostream>
#include <memory>
#include "loki_engine.hpp"

namespace loki::uci
{
	enum class UCI_STATE
	{
		Boot, // engine is running, but has not initialized any position
		Ready, // engine is ready to perform a search
		Searching, // engine is in the middle of searching
		Quit
	};

	class context
	{
	private:
		std::unique_ptr<i_loki_engine> m_owned_engine;

	public:
		i_loki_engine& engine;
		UCI_STATE state;
		std::istream& in;
		std::ostream& out;
		std::ostream& error;

		context(UCI_STATE ctx_state, std::istream& input, std::ostream& output, std::ostream& err)
			: m_owned_engine{ std::make_unique<loki_engine>() }, engine{ *m_owned_engine }, state{ ctx_state }, in{ input }, out{ output }, error{ err }
		{
		}

		context(i_loki_engine& eng, UCI_STATE ctx_state, std::istream& input, std::ostream& output, std::ostream& err)
			: m_owned_engine{ nullptr }, engine{ eng }, state{ ctx_state }, in{ input }, out{ output }, error{ err }
		{
		}
	};
}
