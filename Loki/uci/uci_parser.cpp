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
#include "command_registry.hpp"
#include "context.hpp"
#include <iostream>


namespace loki::uci
{
	uci_parser::uci_parser()
		: context{ UCI_STATE::Boot, std::cin, std::cout, std::cerr }, m_handlers{}
	{
		auto handlers = std::move(command_registry::instance().commands());
		for (auto& handler : handlers)
		{
			auto key = handler->command_name();
			m_handlers.emplace(std::move(key), std::move(handler));
		}
	}

	int uci_parser::uci_loop() noexcept
	{
		try
		{
			std::string command, parameters;		
			while (state != UCI_STATE::Quit)
			{
				try
				{
					in >> command;
					std::getline(in, parameters);
					if (!m_handlers.contains(command))
						continue;

					const auto& handler = m_handlers[command];
					if (!handler->can_execute(this))
						throw_msg<uci_error>("engine is in an invalid state for a '{}' command.", command);

					handler->execute(tokenize(parameters), this);
				}
				catch (const uci_error& e)
				{
					out << "info string error: " << e.what() << std::endl;
					continue;
				}
			}
		}
		catch (const loki_exception& e)
		{
			error << "Internal Error: " << e.what() << std::endl;
			error << "Stack-trace:\n\n" << e.trace() << std::endl;
			return EXIT_FAILURE;
		}
		catch (const std::exception& e)
		{
			error << e.what() << std::endl;
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}

	std::vector<std::string> uci_parser::tokenize(std::string command)
	{
		auto collapsed = util::collapse_whitespace(command);
		auto tokens = util::split(collapsed, ' ');
		return tokens;
	}
}
