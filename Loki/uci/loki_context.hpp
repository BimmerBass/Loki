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
#include <iostream>
#include <optional>
#include <vector>

#include "util/exception.hpp"
#include "position/game_state.hpp"

namespace loki::search
{
	struct limits
	{
		int i;
	};
}

namespace loki::uci
{
	CHILD_EXCEPTION(uci_exception, loki_exception);

	/// <summary>
	/// Main class for interfacing with the Loki chess engine.
	/// Exposes a UCI-like interface and streams potential output to a specified textual stream.
	/// This class exists to make it easy to use Loki in client-code without having to fidget with multi-processing and IO.
	/// </summary>
	class loki_context
	{
	public:
		CHILD_EXCEPTION(not_implemented_error, loki_exception);

		/// <summary>
		/// Initialize a context object
		/// </summary>
		/// <param name="os">The ostream to write results to.</param>
		loki_context(std::ostream& os);

		//
		//	Below functions correspond to the "GUI to engine" bullet-points in 
		//	https://gist.github.com/DOBRO/2592c6dad754ba67e6dcaec8c90165bf
		//
		void uci() const;
		void debug() const;
		void isready() const;
		void setoption(
			std::string name, 
			std::optional<std::string> value);
		void register_() const;
		void ucinewgame();
		void position(
			const position::game_state* newState);
		void go(
			const search::limits* limits);
		void stop();
		void ponderhit();
	private:
		std::ostream& m_os;
	};

}