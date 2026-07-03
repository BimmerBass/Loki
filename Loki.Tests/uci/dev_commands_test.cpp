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
#include "Loki/defs.hpp"
#include "Loki/uci/command_registry.hpp"
#include "Loki/uci/context.hpp"
#include "Loki/uci/uci_parser.hpp"

#ifdef LOKI_ENABLE_DEV_COMMANDS

namespace uci_dev_command_tests
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

	class command_context
	{
	public:
		std::stringstream input;
		std::stringstream output;
		std::stringstream error;
		loki_engine engine;
		context ctx;

		command_context(UCI_STATE state = UCI_STATE::Boot, bool initialize_startpos = false)
			: ctx{ engine, state, input, output, error }
		{
			if (!initialize_startpos)
				return;

			const auto commands = command_registry::instance().commands();
			auto* position = find_command(commands, "position");
			REQUIRE(position != nullptr);

			position->execute(std::vector<std::string>{"startpos"}, &ctx);
			REQUIRE(ctx.state == UCI_STATE::Ready);
		}
	};

	std::string current_fen(const context& ctx)
	{
		return ctx.engine.position()->to_fen();
	}

	i_uci_command* require_command(const std::vector<std::unique_ptr<i_uci_command>>& commands, const std::string& name)
	{
		auto* command = find_command(commands, name);
		REQUIRE(command != nullptr);
		return command;
	}

	TEST_CASE("development commands expose expected capabilities", "[uci][commands][dev]")
	{
		const auto commands = command_registry::instance().commands();

		for (const auto& name : {"move", "undo", "printpos", "printmoves", "perft"})
		{
			SECTION(std::string{name} + " is registered")
			{
				REQUIRE(find_command(commands, name) != nullptr);
			}
		}

		SECTION("position-based commands only execute when the context is ready")
		{
			for (const auto& name : {"move", "undo", "printmoves", "perft"})
			{
				SECTION(name)
				{
					auto* command = require_command(commands, name);
					command_context boot_context{ UCI_STATE::Boot };
					command_context ready_context{ UCI_STATE::Ready };
					command_context searching_context{ UCI_STATE::Searching };
					command_context quit_context{ UCI_STATE::Quit };

					REQUIRE_FALSE(command->can_execute(&boot_context.ctx));
					REQUIRE(command->can_execute(&ready_context.ctx));
					REQUIRE_FALSE(command->can_execute(&searching_context.ctx));
					REQUIRE_FALSE(command->can_execute(&quit_context.ctx));
				}
			}
		}

		SECTION("printpos executes outside boot and search")
		{
			auto* command = require_command(commands, "printpos");
			command_context boot_context{ UCI_STATE::Boot };
			command_context ready_context{ UCI_STATE::Ready };
			command_context searching_context{ UCI_STATE::Searching };
			command_context quit_context{ UCI_STATE::Quit };

			REQUIRE_FALSE(command->can_execute(&boot_context.ctx));
			REQUIRE(command->can_execute(&ready_context.ctx));
			REQUIRE_FALSE(command->can_execute(&searching_context.ctx));
			REQUIRE(command->can_execute(&quit_context.ctx));
		}
	}

	TEST_CASE("move command handles moves", "[uci][commands][dev][move]")
	{
		const auto commands = command_registry::instance().commands();
		auto* move = require_command(commands, "move");

		SECTION("applies legal move")
		{
			command_context test_context{ UCI_STATE::Boot, true };

			move->execute(std::vector<std::string>{"e2e4"}, &test_context.ctx);

			REQUIRE(current_fen(test_context.ctx) == "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
			REQUIRE(test_context.output.str().empty());
			REQUIRE(test_context.error.str().empty());
		}

		SECTION("rejects empty arguments")
		{
			command_context test_context{ UCI_STATE::Boot, true };

			REQUIRE_THROWS_AS(move->execute(std::vector<std::string>{}, &test_context.ctx), uci_parser::uci_error);
		}

		SECTION("rejects illegal move")
		{
			command_context test_context{ UCI_STATE::Boot, true };
			const auto before = current_fen(test_context.ctx);

			REQUIRE_THROWS_AS(move->execute(std::vector<std::string>{"e2e5"}, &test_context.ctx), uci_parser::uci_error);

			REQUIRE(current_fen(test_context.ctx) == before);
		}

		SECTION("rejects move absent from pseudo-legal move list")
		{
			command_context test_context{ UCI_STATE::Boot, true };
			const auto before = current_fen(test_context.ctx);

			REQUIRE_THROWS_AS(move->execute(std::vector<std::string>{"e2e1"}, &test_context.ctx), uci_parser::uci_error);

			REQUIRE(current_fen(test_context.ctx) == before);
		}
	}

	TEST_CASE("undo command reverts the last move", "[uci][commands][dev][undo]")
	{
		const auto commands = command_registry::instance().commands();
		auto* move = require_command(commands, "move");
		auto* undo = require_command(commands, "undo");

		SECTION("reverts last move")
		{
			command_context test_context{ UCI_STATE::Boot, true };
			move->execute(std::vector<std::string>{"e2e4"}, &test_context.ctx);

			undo->execute(std::vector<std::string>{}, &test_context.ctx);

			REQUIRE(current_fen(test_context.ctx) == constants::START_FEN);
			REQUIRE(test_context.output.str().empty());
			REQUIRE(test_context.error.str().empty());
		}

		SECTION("rejects undo with no move history")
		{
			command_context test_context{ UCI_STATE::Boot, true };
			const auto before = current_fen(test_context.ctx);

			REQUIRE_THROWS_AS(undo->execute(std::vector<std::string>{}, &test_context.ctx), uci_parser::uci_error);

			REQUIRE(current_fen(test_context.ctx) == before);
		}
	}

	TEST_CASE("printpos command writes position details", "[uci][commands][dev][printpos]")
	{
		const auto commands = command_registry::instance().commands();
		auto* printpos = require_command(commands, "printpos");

		SECTION("prints board and metadata")
		{
			command_context test_context{ UCI_STATE::Boot, true };

			printpos->execute(std::vector<std::string>{}, &test_context.ctx);
			const auto output = test_context.output.str();

			REQUIRE(output.find("POSITION:") != std::string::npos);
			REQUIRE(output.find("/POSITION") != std::string::npos);
			REQUIRE(output.find("INFORMATION") != std::string::npos);
			REQUIRE(output.find("/INFORMATION") != std::string::npos);
			REQUIRE(output.find(constants::START_FEN) != std::string::npos);
			REQUIRE(output.find("Side to move:") != std::string::npos);
			REQUIRE(output.find("En-passant square:") != std::string::npos);
			REQUIRE(output.find("Castling rights:") != std::string::npos);
			REQUIRE(output.find("Fifty move counter:") != std::string::npos);
			REQUIRE(output.find("Full move counter:") != std::string::npos);
			REQUIRE(test_context.error.str().empty());
		}

		SECTION("does not mutate position")
		{
			command_context test_context{ UCI_STATE::Boot, true };
			const auto before = current_fen(test_context.ctx);

			printpos->execute(std::vector<std::string>{}, &test_context.ctx);

			REQUIRE(current_fen(test_context.ctx) == before);
		}
	}

	TEST_CASE("printmoves command writes generated move lists", "[uci][commands][dev][printmoves]")
	{
		const auto commands = command_registry::instance().commands();
		auto* printmoves = require_command(commands, "printmoves");

		SECTION("prints all moves by default")
		{
			command_context test_context{ UCI_STATE::Boot, true };
			const auto before = current_fen(test_context.ctx);

			printmoves->execute(std::vector<std::string>{}, &test_context.ctx);
			const auto output = test_context.output.str();

			REQUIRE(output.find("all moves for position:") != std::string::npos);
			REQUIRE(output.find("[1]") != std::string::npos);
			REQUIRE(current_fen(test_context.ctx) == before);
		}

		SECTION("prints explicit all moves")
		{
			command_context test_context{ UCI_STATE::Boot, true };
			const auto before = current_fen(test_context.ctx);

			printmoves->execute(std::vector<std::string>{"all"}, &test_context.ctx);
			const auto output = test_context.output.str();

			REQUIRE(output.find("all moves for position:") != std::string::npos);
			REQUIRE(output.find("[1]") != std::string::npos);
			REQUIRE(current_fen(test_context.ctx) == before);
		}

		SECTION("prints quiet moves")
		{
			command_context test_context{ UCI_STATE::Boot, true };
			const auto before = current_fen(test_context.ctx);

			printmoves->execute(std::vector<std::string>{"quiet"}, &test_context.ctx);
			const auto output = test_context.output.str();

			REQUIRE(output.find("quiet moves for position:") != std::string::npos);
			REQUIRE(output.find("[1]") != std::string::npos);
			REQUIRE(current_fen(test_context.ctx) == before);
		}

		SECTION("prints active moves")
		{
			command_context test_context{ UCI_STATE::Boot, true };
			const auto before = current_fen(test_context.ctx);

			printmoves->execute(std::vector<std::string>{"active"}, &test_context.ctx);

			REQUIRE(test_context.output.str().find("active moves for position:") != std::string::npos);
			REQUIRE(current_fen(test_context.ctx) == before);
		}

		SECTION("rejects invalid selector")
		{
			command_context test_context{ UCI_STATE::Boot, true };
			const auto before = current_fen(test_context.ctx);

			REQUIRE_THROWS_AS(printmoves->execute(std::vector<std::string>{"captures"}, &test_context.ctx), uci_parser::uci_error);

			REQUIRE(current_fen(test_context.ctx) == before);
		}
	}

	TEST_CASE("perft command writes perft report without mutating state", "[uci][commands][dev][perft]")
	{
		const auto commands = command_registry::instance().commands();
		auto* perft = require_command(commands, "perft");

		SECTION("prints perft report")
		{
			command_context test_context{ UCI_STATE::Boot, true };

			perft->execute(std::vector<std::string>{"1"}, &test_context.ctx);
			const auto output = test_context.output.str();

			REQUIRE(output.find("<PERFT TEST FOR DEPTH = 1>") != std::string::npos);
			REQUIRE(output.find(std::string{ "FEN: " } + constants::START_FEN) != std::string::npos);
			REQUIRE(output.find("[1]") != std::string::npos);
			REQUIRE(output.find("--->") != std::string::npos);
			REQUIRE(output.find("nodes.") != std::string::npos);
			REQUIRE(output.find("Perft test completed after:") != std::string::npos);
			REQUIRE(output.find("Nodes/second:") != std::string::npos);
			REQUIRE(output.find("Nodes visited:") != std::string::npos);
			REQUIRE(output.find("</PERFT TEST FOR DEPTH = 1>") != std::string::npos);
		}

		SECTION("does not mutate position")
		{
			command_context test_context{ UCI_STATE::Boot, true };
			const auto before = current_fen(test_context.ctx);

			perft->execute(std::vector<std::string>{"1"}, &test_context.ctx);

			REQUIRE(current_fen(test_context.ctx) == before);
		}

		SECTION("rejects missing depth")
		{
			command_context test_context{ UCI_STATE::Boot, true };
			const auto before = current_fen(test_context.ctx);

			REQUIRE_THROWS_AS(perft->execute(std::vector<std::string>{}, &test_context.ctx), uci_parser::uci_error);

			REQUIRE(current_fen(test_context.ctx) == before);
		}
	}
}

#endif
