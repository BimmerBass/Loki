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
#include "uci_parser.hpp"
#include "util/stringops.hpp"
#include <iostream>

namespace loki::uci
{
	uci_parser::uci_parser(const std::shared_ptr<context_interface>& ptr)
		: m_context{ ptr }
	{
		// register the callbacks for the UCI commands.
#define REGISTER_CALLBACK(func) m_uciCallbacks[std::string(#func).substr(6)] = [this](const tokens_t& s) { func(s);}
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
		REGISTER_CALLBACK(parse_stop);
		REGISTER_CALLBACK(parse_printpos);
		REGISTER_CALLBACK(parse_perft);
#undef REGISTER_CALLBACK
	}

	int uci_parser::uci_loop() noexcept
	{
		try
		{
			for (std::string line; std::getline(std::cin, line);)
			{
				auto tokens = util::split(line, ' ');
				if (tokens.size() == 0)
					continue;

				const auto& command = tokens[0];
				if (m_uciCallbacks.find(command) != m_uciCallbacks.end())
					m_uciCallbacks[command](std::vector(tokens.begin() + 1, tokens.end()));
			}
		}
		catch (const quit_exception&)
		{
			return EXIT_SUCCESS;
		}
		catch (const loki_exception& e)
		{
			std::cerr << "Internal Error: " << e.what() << std::endl;
			std::cerr << "Stack-trace:\n\n" << e.trace() << std::endl;
			return EXIT_FAILURE;
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}

	void uci_parser::parse_setoption(const tokens_t& tokens)
	{
		// The name and value of the option in <id> should not be case sensitive and can inlude spaces.
		// The substrings "value" and "name" should be avoided in <id> and <x> to allow unambiguous parsing,
		// for example do not use <name> = "draw value".
		if (tokens.size() < 2 || tokens[0] != "name")
		{
			throw_msg<uci_error>("setoption needs at least one 'name' parameter.");
		}
		
		auto i = tokens.begin() + 1;
		std::string name = *i;
		while (++i != tokens.end() && (*i) != "value")
		{
			name += " " + (*i);
		}
		std::optional<std::string> value = std::nullopt;
		if (i != tokens.end() && i+1 != tokens.end() && (*i) == "value")
		{
			value = util::lowercase(*++i);
		}

		m_context->setoption(util::lowercase(name), value);
	}

	void uci_parser::parse_position(const tokens_t& tokens)
	{
		if (tokens.size() < 1)
			throw_msg<uci_error>("position needs at least one parameter.");
		
		std::string fen = constants::START_FEN;
		std::vector<std::string> moves;

		auto it = tokens.begin();
		if ((*it) == "fen")
		{
			fen = *++it;
			while (++it != tokens.end() && (*it) != "moves")
				fen += " " + *it;
		}

		if ((it = std::find(tokens.begin(), tokens.end(), "moves")) != tokens.end())
		{
			while (++it != tokens.end())
				moves.push_back(*it);
		}

		m_context->position(fen, moves);
	}

	void uci_parser::parse_go(const tokens_t&)
	{
		m_context->go(nullptr);
	}

	void uci_parser::parse_printpos(const tokens_t&)
	{
		m_context->printpos();
	}

	void uci_parser::parse_perft(const tokens_t& tokens)
	{
		if (tokens.size() < 1)
			throw_msg<uci_error>("perft needs either a \"depth [depth]\" or non-zero numeric parameter");

		auto it = tokens.begin();
		std::string sdepth;
		if ((*it) == "depth")
		{
			sdepth = *++it;
		}
		else
		{
			sdepth = *it;
		}
		size_t depth = std::stol(sdepth);
		m_context->perft(depth);
	}
}