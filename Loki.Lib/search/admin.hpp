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
	/// search_admin holds all data that is to be shared between the searchers. This will include things like slider generator, transposition table and so on.
	/// It is also responsible for managing the engine's options.
	/// 
	/// This makes search_admin, in essence, the engine itself.
	/// </summary>
	class search_admin
	{
		EXCEPTION_CLASS(e_Search, e_lokiError);
	public:
		static constexpr const char* START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq";
	
	private:
		std::unique_ptr<main_thread> m_mainThread;
		position::game_state m_state;
		movegen::move_list_t m_legal_moves;

		// "Global" resources.
		movegen::magics::slider_generator_t m_slider_generator;
		evaluation::evaluation_params_t m_eval_parameters;
		std::unique_ptr<const options_manager> m_optManager;
		std::shared_ptr<transposition_table> m_hashTable;
	public:
		search_admin();

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
		void set_position(std::string fen, const std::vector<std::string>& moves);

		/// <summary>
		/// Set a registered option.
		/// </summary>
		void set_option(const std::string& name, const std::string& value);

		/// <summary>
		/// This is the main search function of Loki, and will be called by the engine manager.
		/// </summary>
		/// <param name="limits"></param>
		void search(std::shared_ptr<search_limits>& limits);

		/// <summary>
		/// Perform a perft test on the current position.
		/// </summary>
		void do_perft(eDepth d);

		/// <summary>
		/// Get a list of all option string that we should print to the uci.
		/// </summary>
		std::vector<std::string> get_options() const;

		// Return legal moves.
		inline const movegen::move_list_t& legal_moves() const noexcept { return m_legal_moves; }
		inline const position::game_state& game_state() const noexcept { return m_state; }
	private:
		void generate_legals(position::position_t pos);
		options_manager* register_options() const;
	};

}