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

namespace loki::search
{
	/// <summary>
	/// search_context holds all data that is to be shared between the searchers. This will include things like slider generator, transposition table and so on.
	/// It is also responsible for managing the engine's options.
	/// 
	/// This makes search_context, in essence, the engine itself.
	/// </summary>
	class search_context
	{
		EXCEPTION_CLASS(e_Search, e_lokiError);
	public:
		static constexpr const char* START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq";
	public:
		search_context();

		/// <summary>
		/// Will load the starting FEN and clear all search-related data structures.
		/// </summary>
		void reset();

		/// <summary>
		/// Will set m_state with the given FEN, but won't execute the moves before "go" is received
		/// This is done to make pondering possible.
		/// </summary>
		/// <param name="fen"></param>
		/// <param name="moves"></param>
		void set_position(const std::string& fen, const std::vector<std::string>& moves);

		// Sets an option
		void set_option(const std::string& name, const std::string& value) {};

		/// <summary>
		/// This is the main search function of Loki, and will be called by the engine manager.
		/// </summary>
		/// <param name="limits"></param>
		void search(search_limits limits) {};

		// Return legal moves.
		inline const movegen::move_list_t& legal_moves() const noexcept { return m_legal_moves; }
	private:
		std::vector<std::string> m_movesToMake;
		position::game_state m_state;
		movegen::magics::slider_generator_t m_slider_generator;
		movegen::move_list_t m_legal_moves;

	private:
		void generate_legals();
	};

}