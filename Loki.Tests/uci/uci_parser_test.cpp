// Loki, a UCI-compliant chess playing software
// Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
// Loki is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Loki is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "pch.hpp"
#include "Loki/uci/command_registry.hpp"
#include "Loki/uci/context.hpp"
#include "Loki/position/game_state.hpp"
#include "Loki/uci/uci_parser.hpp"

namespace uci_tests
{
	using namespace loki;
	using namespace loki::uci;

	i_uci_command* find_command(const std::vector<std::unique_ptr<i_uci_command>>& commands, const std::string& name)
	{
		for (const auto& command : commands)
		{
			if (command->command_name() == name)
				return command.get();
		}
		return nullptr;
	}

	TEST_CASE("command registry exposes the active UCI commands", "[uci][registry]")
	{
		const auto commands = command_registry::instance().commands();
		std::vector<std::string> names;
		names.reserve(commands.size());
		for (const auto& command : commands)
			names.push_back(command->command_name());

		REQUIRE(std::find(names.begin(), names.end(), "uci") != names.end());
		REQUIRE(std::find(names.begin(), names.end(), "isready") != names.end());
		REQUIRE(std::find(names.begin(), names.end(), "position") != names.end());
		REQUIRE(std::find(names.begin(), names.end(), "quit") != names.end());
	}

#ifdef LOKI_ENABLE_DEV_COMMANDS
	TEST_CASE("development commands are registered when dev commands are enabled", "[uci][registry][dev]")
	{
		const auto commands = command_registry::instance().commands();
		std::vector<std::string> names;
		names.reserve(commands.size());
		for (const auto& command : commands)
			names.push_back(command->command_name());

		REQUIRE(std::find(names.begin(), names.end(), "debug") != names.end());
		REQUIRE(std::find(names.begin(), names.end(), "perft") != names.end());
		REQUIRE(std::find(names.begin(), names.end(), "printpos") != names.end());
	}
#endif

	TEST_CASE("uci_parser dispatches commands from the active stream buffers", "[uci][parser]")
	{
		std::istringstream input("isready\nquit\n");
		std::ostringstream output;
		std::ostringstream error;

		auto* cin_buf = std::cin.rdbuf(input.rdbuf());
		auto* cout_buf = std::cout.rdbuf(output.rdbuf());
		auto* cerr_buf = std::cerr.rdbuf(error.rdbuf());

		auto restore = [&]()
		{
			std::cin.rdbuf(cin_buf);
			std::cout.rdbuf(cout_buf);
			std::cerr.rdbuf(cerr_buf);
		};

		try
		{
			uci_parser parser;
			REQUIRE(parser.uci_loop() == EXIT_SUCCESS);
		}
		catch (...)
		{
			restore();
			throw;
		}
		restore();

		REQUIRE(output.str().find("readyok") != std::string::npos);
		REQUIRE(error.str().empty());
	}

	TEST_CASE("uci command writes its banner", "[uci][commands][uci]")
	{
		const auto commands = command_registry::instance().commands();
		auto uci = find_command(commands, "uci");
		REQUIRE(uci != nullptr);

		std::stringstream input;
		std::stringstream output;
		std::stringstream error;
		loki_engine engine;
		context ctx{engine, UCI_STATE::Boot, input, output, error};

		uci->execute(std::vector<std::string>{}, &ctx);
		const auto uci_output = output.str();
		REQUIRE(uci_output.find("id name ") == 0);
		REQUIRE(uci_output.find("id author ") != std::string::npos);
		REQUIRE(uci_output.find("uciok") != std::string::npos);
	}

	TEST_CASE("isready command writes readyok", "[uci][commands][isready]")
	{
		const auto commands = command_registry::instance().commands();
		auto* isready = find_command(commands, "isready");
		REQUIRE(isready != nullptr);

		std::stringstream input;
		std::stringstream output;
		std::stringstream error;
		loki_engine engine;
		context ctx{engine, UCI_STATE::Boot, input, output, error};

		isready->execute(std::vector<std::string>{}, &ctx);
		REQUIRE(output.str() == "readyok\n");
	}

	TEST_CASE("position command applies startpos and moves", "[uci][commands][position]")
	{
		const auto commands = command_registry::instance().commands();
		auto* position = find_command(commands, "position");
		REQUIRE(position != nullptr);

		std::stringstream input;
		std::stringstream output;
		std::stringstream error;
		loki_engine engine;
		context ctx{engine, UCI_STATE::Boot, input, output, error};

		position->execute(std::vector<std::string>{"startpos", "moves", "e2e4", "e7e5"}, &ctx);
		REQUIRE(ctx.state == UCI_STATE::Ready);

		const auto& state = ctx.engine.position()->make_view()->game_state();
		REQUIRE(loki::position::game_state::to_fen(std::make_shared<loki::position::game_state>(*state))
			== "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2");
	}

	TEST_CASE("quit command transitions the context to Quit", "[uci][commands][quit]")
	{
		const auto commands = command_registry::instance().commands();
		auto* quit = find_command(commands, "quit");
		REQUIRE(quit != nullptr);

		std::stringstream input;
		std::stringstream output;
		std::stringstream error;
		loki_engine engine;
		context ctx{engine, UCI_STATE::Boot, input, output, error};

		quit->execute(std::vector<std::string>{}, &ctx);
		REQUIRE(ctx.state == UCI_STATE::Quit);
	}

	TEST_CASE("unsupported commands still throw not_implemented_error", "[uci][commands][stubbed]")
	{
		auto commands = command_registry::instance().commands();
		std::stringstream input;
		std::stringstream output;
		std::stringstream error;
		loki_engine engine;
		context ctx{engine, UCI_STATE::Ready, input, output, error};

		for (const auto& name : {"go", "register", "setoption", "stop", "ponderhit", "ucinewgame", "debug"})
		{
			SECTION(name)
			{
				auto* command = find_command(commands, name);
				REQUIRE(command != nullptr);
				REQUIRE_THROWS_AS(command->can_execute(&ctx), not_implemented_error);
				REQUIRE_THROWS_AS(command->execute(std::vector<std::string>{}, &ctx), not_implemented_error);
			}
		}
	}
}
