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

	class search_spy_engine final : public i_loki_engine
	{
	public:
		mutable std::optional<search::limits> searched_limits;
		size_t clear_count = 0;

		bool set_position(const position::game_state& state, const std::vector<movegen::move>& moves) override
		{
			return m_engine.set_position(state, moves);
		}

		void set_position(const position::game_state& state) override
		{
			m_engine.set_position(state);
		}

		void set_position(position::search_position_t state) override
		{
			m_engine.set_position(std::move(state));
		}

		position::search_position_t make_position(const position::game_state& state) const override
		{
			return m_engine.make_position(state);
		}

		void clear() override
		{
			++clear_count;
			m_engine.clear();
		}

		void search(const search::limits limits, search::search_thread::callback_t) override
		{
			searched_limits = limits;
		}

		void stop_search(bool wait = false) override
		{
			m_engine.stop_search(wait);
		}

		size_t perft(size_t depth, std::ostream& out) const override
		{
			return m_engine.perft(depth, out);
		}

		const position::search_position_t& position() const override
		{
			return m_engine.position();
		}

	private:
		loki_engine m_engine;
	};

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

	TEST_CASE("ucinewgame command clears engine state and sets the starting position", "[uci][commands][ucinewgame]")
	{
		const auto commands = command_registry::instance().commands();
		auto* position = find_command(commands, "position");
		auto* ucinewgame = find_command(commands, "ucinewgame");
		REQUIRE(position != nullptr);
		REQUIRE(ucinewgame != nullptr);

		std::stringstream input;
		std::stringstream output;
		std::stringstream error;
		search_spy_engine engine;
		context ctx{ engine, UCI_STATE::Boot, input, output, error };

		position->execute(std::vector<std::string>{ "startpos", "moves", "e2e4", "e7e5" }, &ctx);
		REQUIRE(ctx.state == UCI_STATE::Ready);
		REQUIRE(ucinewgame->can_execute(&ctx));

		ucinewgame->execute(std::vector<std::string>{}, &ctx);

		REQUIRE(ctx.state == UCI_STATE::Ready);
		REQUIRE(engine.clear_count == 1);

		const auto& state = ctx.engine.position()->make_view()->game_state();
		REQUIRE(loki::position::game_state::to_fen(std::make_shared<loki::position::game_state>(*state))
			== constants::START_FEN);
	}

	TEST_CASE("go command parses UCI search parameters", "[uci][commands][go]")
	{
		const auto commands = command_registry::instance().commands();
		auto* position = find_command(commands, "position");
		auto* go = find_command(commands, "go");
		REQUIRE(position != nullptr);
		REQUIRE(go != nullptr);

		std::stringstream input;
		std::stringstream output;
		std::stringstream error;
		search_spy_engine engine;
		context ctx{engine, UCI_STATE::Boot, input, output, error};

		position->execute(std::vector<std::string>{"startpos"}, &ctx);
		REQUIRE(ctx.state == UCI_STATE::Ready);

		auto execute_go = [&](std::vector<std::string> tokens) -> const search::limits&
		{
			REQUIRE(go->can_execute(&ctx));
			go->execute(std::move(tokens), &ctx);
			REQUIRE(ctx.state == UCI_STATE::Searching);
			REQUIRE(engine.searched_limits.has_value());
			return *engine.searched_limits;
		};

		SECTION("can_execute only allows ready contexts")
		{
			context boot_ctx{ engine, UCI_STATE::Boot, input, output, error };
			context searching_ctx{ engine, UCI_STATE::Searching, input, output, error };
			context quit_ctx{ engine, UCI_STATE::Quit, input, output, error };

			REQUIRE_FALSE(go->can_execute(&boot_ctx));
			REQUIRE(go->can_execute(&ctx));
			REQUIRE_FALSE(go->can_execute(&searching_ctx));
			REQUIRE_FALSE(go->can_execute(&quit_ctx));
		}

		SECTION("parses searchmoves")
		{
			const auto& limits = execute_go({ "searchmoves", "e2e4", "d2d4" });

			REQUIRE(limits.searchmoves.size() == 2);
			REQUIRE(limits.searchmoves[0].to_string() == "e2e4");
			REQUIRE(limits.searchmoves[1].to_string() == "d2d4");
		}

		SECTION("parses ponder")
		{
			const auto& limits = execute_go({ "ponder" });

			REQUIRE(limits.pondering);
		}

		SECTION("parses wtime")
		{
			const auto& limits = execute_go({ "wtime", "60000" });

			REQUIRE(limits.wtime.has_value());
			REQUIRE(std::get<0>(*limits.wtime) == 60000);
			REQUIRE(std::get<1>(*limits.wtime) == 0);
		}

		SECTION("parses btime")
		{
			const auto& limits = execute_go({ "btime", "45000" });

			REQUIRE(limits.btime.has_value());
			REQUIRE(std::get<0>(*limits.btime) == 45000);
			REQUIRE(std::get<1>(*limits.btime) == 0);
		}

		SECTION("parses winc")
		{
			const auto& limits = execute_go({ "winc", "1000" });

			REQUIRE(limits.wtime.has_value());
			REQUIRE(std::get<0>(*limits.wtime) == 0);
			REQUIRE(std::get<1>(*limits.wtime) == 1000);
		}

		SECTION("parses binc")
		{
			const auto& limits = execute_go({ "binc", "500" });

			REQUIRE(limits.btime.has_value());
			REQUIRE(std::get<0>(*limits.btime) == 0);
			REQUIRE(std::get<1>(*limits.btime) == 500);
		}

		SECTION("parses movestogo")
		{
			const auto& limits = execute_go({ "movestogo", "20" });

			REQUIRE(limits.movestogo.has_value());
			REQUIRE(*limits.movestogo == 20);
		}

		SECTION("parses depth")
		{
			const auto& limits = execute_go({ "depth", "12" });

			REQUIRE(limits.depth.has_value());
			REQUIRE(*limits.depth == 12);
		}

		SECTION("parses nodes")
		{
			const auto& limits = execute_go({ "nodes", "100000" });

			REQUIRE(limits.nodes.has_value());
			REQUIRE(*limits.nodes == 100000);
		}

		SECTION("parses mate")
		{
			const auto& limits = execute_go({ "mate", "3" });

			REQUIRE(limits.mate.has_value());
			REQUIRE(*limits.mate == 3);
		}

		SECTION("parses movetime")
		{
			const auto& limits = execute_go({ "movetime", "2500" });

			REQUIRE(limits.movetime.has_value());
			REQUIRE(std::get<0>(*limits.movetime) == 2500);
			REQUIRE(std::get<1>(*limits.movetime) == 0);
		}

		SECTION("parses infinite")
		{
			const auto& limits = execute_go({ "infinite" });

			REQUIRE(limits.infinite);
		}

		SECTION("parses combined options")
		{
			const auto& limits = execute_go({
				"searchmoves", "e2e4", "d2d4",
				"ponder",
				"wtime", "60000",
				"btime", "45000",
				"winc", "1000",
				"binc", "500",
				"movestogo", "20",
				"depth", "12",
				"nodes", "100000",
				"mate", "3",
				"movetime", "2500",
				"infinite"
			});

			REQUIRE(limits.searchmoves.size() == 2);
			REQUIRE(limits.searchmoves[0].to_string() == "e2e4");
			REQUIRE(limits.searchmoves[1].to_string() == "d2d4");
			REQUIRE(limits.pondering);
			REQUIRE(limits.wtime.has_value());
			REQUIRE(std::get<0>(*limits.wtime) == 60000);
			REQUIRE(std::get<1>(*limits.wtime) == 1000);
			REQUIRE(limits.btime.has_value());
			REQUIRE(std::get<0>(*limits.btime) == 45000);
			REQUIRE(std::get<1>(*limits.btime) == 500);
			REQUIRE(limits.movestogo.has_value());
			REQUIRE(*limits.movestogo == 20);
			REQUIRE(limits.depth.has_value());
			REQUIRE(*limits.depth == 12);
			REQUIRE(limits.nodes.has_value());
			REQUIRE(*limits.nodes == 100000);
			REQUIRE(limits.mate.has_value());
			REQUIRE(*limits.mate == 3);
			REQUIRE(limits.movetime.has_value());
			REQUIRE(std::get<0>(*limits.movetime) == 2500);
			REQUIRE(std::get<1>(*limits.movetime) == 0);
			REQUIRE(limits.infinite);
		}

		SECTION("accepts searchmoves after other options")
		{
			const auto& limits = execute_go({ "depth", "5", "searchmoves", "g1f3" });

			REQUIRE(limits.depth.has_value());
			REQUIRE(*limits.depth == 5);
			REQUIRE(limits.searchmoves.size() == 1);
			REQUIRE(limits.searchmoves[0].to_string() == "g1f3");
		}

		SECTION("rejects unknown option")
		{
			REQUIRE_THROWS_AS(go->execute(std::vector<std::string>{"fast"}, &ctx), uci_parser::uci_error);
			REQUIRE_THROWS_AS(go->execute(std::vector<std::string>{"depth", "4", "fast"}, &ctx), uci_parser::uci_error);
		}

		SECTION("rejects empty command")
		{
			REQUIRE_THROWS_AS(go->execute(std::vector<std::string>{}, &ctx), uci_parser::uci_error);
		}

		SECTION("rejects missing numeric value")
		{
			for (const auto& option : { "wtime", "btime", "winc", "binc", "movestogo", "depth", "nodes", "mate", "movetime" })
			{
				INFO(option);
				REQUIRE_THROWS_AS(go->execute(std::vector<std::string>{ option }, &ctx), uci_parser::uci_error);
			}
		}

		SECTION("rejects numeric option followed by another option")
		{
			for (const auto& option : { "wtime", "btime", "winc", "binc", "movestogo", "depth", "nodes", "mate", "movetime" })
			{
				INFO(option);
				REQUIRE_THROWS_AS(go->execute(std::vector<std::string>{ option, "infinite" }, &ctx), uci_parser::uci_error);
			}
		}

		SECTION("rejects malformed numeric value")
		{
			for (const auto& value : { "12x", "abc", "-1", "1.5", "999999999999999999999999999999" })
			{
				INFO(value);
				REQUIRE_THROWS_AS(go->execute(std::vector<std::string>{ "depth", value }, &ctx), uci_parser::uci_error);
			}
		}

		SECTION("rejects searchmoves without moves")
		{
			REQUIRE_THROWS_AS(go->execute(std::vector<std::string>{"searchmoves"}, &ctx), uci_parser::uci_error);
			REQUIRE_THROWS_AS(go->execute(std::vector<std::string>{"searchmoves", "depth", "4"}, &ctx), uci_parser::uci_error);
		}

		SECTION("rejects invalid searchmove")
		{
			REQUIRE_THROWS_AS(go->execute(std::vector<std::string>{"searchmoves", "notamove"}, &ctx), uci_parser::uci_error);
			REQUIRE_THROWS_AS(go->execute(std::vector<std::string>{"searchmoves", "e7e5"}, &ctx), uci_parser::uci_error);
			REQUIRE_THROWS_AS(go->execute(std::vector<std::string>{"searchmoves", "e2e5"}, &ctx), uci_parser::uci_error);
		}

		SECTION("rejects duplicate parameters")
		{
			for (const auto& option : { "searchmoves", "ponder", "wtime", "btime", "winc", "binc", "movestogo", "depth", "nodes", "mate", "movetime", "infinite" })
			{
				INFO(option);
				if (std::string_view{ option } == "searchmoves")
				{
					REQUIRE_THROWS_AS(go->execute(std::vector<std::string>{ "searchmoves", "e2e4", "searchmoves", "d2d4" }, &ctx), uci_parser::uci_error);
				}
				else if (std::string_view{ option } == "ponder" || std::string_view{ option } == "infinite")
				{
					REQUIRE_THROWS_AS(go->execute(std::vector<std::string>{ option, option }, &ctx), uci_parser::uci_error);
				}
				else
				{
					REQUIRE_THROWS_AS(go->execute(std::vector<std::string>{ option, "1", option, "2" }, &ctx), uci_parser::uci_error);
				}
			}
		}

		SECTION("rejects malformed value after combined options")
		{
			REQUIRE_THROWS_AS(
				go->execute(std::vector<std::string>{ "searchmoves", "e2e4", "d2d4", "wtime", "60000", "depth", "nope" }, &ctx),
				uci_parser::uci_error);
		}
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

		for (const auto& name : {"register", "setoption", "ponderhit", "debug"})
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
