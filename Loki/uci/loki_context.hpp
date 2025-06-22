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
#include "context_interface.hpp"
#include "movegen/magics/magic_index.hpp"

namespace loki::uci
{
	CHILD_EXCEPTION(uci_exception, loki_exception);

	/// <summary>
	/// Main class for interfacing with the Loki chess engine.
	/// Exposes a UCI-like interface and streams potential output to a specified textual stream.
	/// This class exists to make it easy to use Loki in client-code without having to fidget with multi-processing and IO.
	/// </summary>
	class loki_context : public context_interface
	{
	public:
		/// <summary>
		/// Initialize a context object
		/// </summary>
		/// <param name="os">The ostream to write results to.</param>
		loki_context(std::ostream& os);

		void uci() const override;
		void debug() const override;
		void isready() const override;
		void setoption(
			std::string name, 
			std::optional<std::string> value) override;
		void register_() const override;
		void ucinewgame() override;
		void position(
			const std::string& fen,
			const std::vector<std::string>& moves) override;
		void go(
			const search::limits* limits) override;
		void stop() override;
		void ponderhit() override;
		void printpos() const override;

		inline const position::game_state_t& game_state() const { return m_gamestate; }
	private:
		std::ostream& m_os;
		position::game_state_t m_gamestate;
		movegen::magics::magic_index_t m_rook_index;
		movegen::magics::magic_index_t m_bishop_index;
	};

}