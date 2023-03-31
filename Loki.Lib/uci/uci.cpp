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
#include "loki.pch.hpp"

namespace loki::uci
{
#define REGISTER_CALLBACK(func) m_callbackMap[std::string(#func).substr(6)] = [this](const std::string& s) { func(s);}

	engine_manager::engine_manager() : has_position(false)
	{
		REGISTER_CALLBACK(parse_uci);
		REGISTER_CALLBACK(parse_debug);
		REGISTER_CALLBACK(parse_isready);
		REGISTER_CALLBACK(parse_setoption);
		REGISTER_CALLBACK(parse_register);
		REGISTER_CALLBACK(parse_ucinewgame);
		REGISTER_CALLBACK(parse_position);
		REGISTER_CALLBACK(parse_go);
		REGISTER_CALLBACK(parse_ponderhit);
		REGISTER_CALLBACK(parse_quit);

		REGISTER_CALLBACK(parse_perft);
		REGISTER_CALLBACK(parse_printpos);
	}
#undef REGISTER_CALLBACK

	/// <summary>
	/// Run the command parsing loop responsible for communicating over the UCI protocol (ref: https://www.wbec-ridderkerk.nl/html/UCIProtocol.html)
	/// </summary>
	/// <returns>true if no unexpected error occurs, false otherwise</returns>
	bool engine_manager::run() noexcept
	{
		try
		{
			std::string command;

			while (std::getline(std::cin, command))
			{
				if (command == "\n" || command.empty())
					continue;

				auto token_length = command.find_first_of(' ');
				auto first_token = textutil::lowercase(token_length == std::string::npos ?
					command : command.substr(0, token_length));

				if (m_callbackMap.find(first_token) != m_callbackMap.end())
					m_callbackMap[first_token](command);
			}
		}
		catch (const e_quitException&)
		{
			return true;
		}
		catch (const std::exception& e)
		{
			// Output the exception message as a UCI info command.
			std::cout << "info string Exception thrown: " << e.what() << std::endl;
			return false;
		}

		return true;
	}

	void engine_manager::parse_uci(const std::string& /* unused */)
	{
		std::cout << "id name " << sOptions["NAME"] << " " << sOptions["VERSION"] << std::endl;
		std::cout << "id author " << sOptions["AUTHOR"] << std::endl;
		auto opts = m_admin.get_options();
		for (auto& opt : opts)
			std::cout << opt << std::endl;

		std::cout << "uciok" << std::endl;
	}

	void engine_manager::parse_isready(const std::string& /* unused */)
	{
		std::cout << "readyok" << std::endl;
	}

	void engine_manager::parse_setoption(const std::string& cmd)
	{
		std::string name = "", value = "", token = "";
		std::stringstream ss(cmd);
		ss >> token >> token;
		while (ss >> token && token != "value")
			name += (name.size() > 0 ? " " : "") + token;
		while (ss >> token)
			value += (value.size() > 0 ? " " : "") + token;
		m_admin.set_option(name, value);
	}

	void engine_manager::parse_ucinewgame(const std::string& /* unused */)
	{
		m_admin.reset();
	}

	void engine_manager::parse_position(const std::string& cmd)
	{
		size_t pos = std::string::npos;
		std::string fen;
		std::vector<std::string> moves;
		if (cmd.find("startpos") != std::string::npos)
			fen = m_admin.START_FEN;
		else if ((pos = cmd.find("fen ")) != std::string::npos)
		{
			auto f_start = pos + 4;
			auto f_end = cmd.find(" moves ");
			fen = cmd.substr(f_start, f_end - f_start);
		}
		else
			return;
		if ((pos = cmd.find(" moves ")) != std::string::npos)
		{
			auto moves_start = pos + 7;
			moves = textutil::split(cmd.substr(moves_start));
		}

		m_admin.set_position(fen, moves);
		has_position = true;
	}

	void engine_manager::parse_go(const std::string& cmd)
	{
		if (!has_position)
			throw e_engineManager("A 'go' command was received before the first 'position command'");
		auto limits = std::make_shared<search::search_limits>();
		std::string tk;
		std::stringstream ss(cmd);

		while (ss >> tk)
		{
			// NOTE: The "mate" command has intentionally not been implemented due to its rare usage.
			if (tk == "wtime") ss >> limits->time[WHITE];
			else if (tk == "btime") ss >> limits->time[BLACK];
			else if (tk == "winc") ss >> limits->inc[WHITE];
			else if (tk == "binc") ss >> limits->inc[BLACK];
			else if (tk == "movestogo") ss >> limits->movestogo;
			else if (tk == "depth") ss >> limits->depth;
			else if (tk == "nodes") ss >> limits->nodes;
			else if (tk == "movetime") ss >> limits->movetime;
			else if (tk == "infinite") limits->infinite = true;
			else if (tk == "ponder") limits->ponder = true;
			else if (tk == "searchmoves")
			{
				std::stringstream tmp(cmd);
				move_t move;
				while (tmp >> tk && tk != "searchmoves") continue;

				while (tmp >> tk &&
					movegen::is_algebraic_move(tk) &&
					(move = m_admin.legal_moves().find(tk)) != MOVE_NULL)
				{
					limits->searchmoves.add(move, VALUE_ZERO);
				}
			}
		}
		m_admin.search(limits);
	}

	void engine_manager::parse_perft(const std::string& cmd)
	{
		if (!has_position)
			throw e_engineManager("A 'perft' command was received before the first 'position command'");

		long depth = 1;
		size_t pos;
		if ((pos = cmd.find(" depth ")) != std::string::npos)
			depth = std::stol(cmd.substr(pos + 7, std::string::npos));

		if (depth <= 0)
			throw e_engineManager("'depth' was less than or equal to zero");

		m_admin.do_perft(static_cast<eDepth>(depth));
	}

	void engine_manager::parse_printpos(const std::string& /* unused */)
	{
		if (!has_position)
			throw e_engineManager("A 'printpos' command was received before the first 'position command'");

		std::cout << m_admin.game_state() << std::endl;
	}
}