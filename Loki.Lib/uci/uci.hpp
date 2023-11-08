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


namespace loki::uci
{
	class engine_manager
	{
		EXCEPTION_CLASS(e_engineManager, e_lokiError);
	public:
		EXCEPTION_CLASS(e_quitException, e_engineManager);
	private:
		inline static std::map<std::string, std::string> sOptions = {
			{ "VERSION", "4.0" },
			{ "NAME", "Loki" },
			{ "AUTHOR", "Niels Abildskov" }
		};

		std::map<std::string, std::function<void(const std::string&)>> m_callbackMap;
		search::search_admin m_admin;
	public:
		engine_manager();

		// Run the UCI loop and return true upon succesful completion.
		bool run() noexcept;
	private:
		void parse_uci(const std::string& cmd);
		void parse_isready(const std::string& cmd);
		void parse_setoption(const std::string& cmd);
		void parse_ucinewgame(const std::string& cmd);
		void parse_position(const std::string& cmd);
		void parse_go(const std::string& cmd);
		void parse_quit(const std::string& /* unused */)
		{
			throw e_quitException();
		}

		void parse_perft(const std::string& cmd);
		void parse_printpos(const std::string& cmd);

		// Parse methods that are intentionally not implemented, but may be in the future.
		void parse_debug(const std::string& /* unused */) {};
		void parse_register(const std::string& /* unused */) {};
		void parse_ponderhit(const std::string& /* unused */) { throw e_notImplementedError(FORMAT_EXCEPTION_MESSAGE("Pondering is not (yet) implemented!")); };
	private:
		bool has_position;
	};

}