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

#include "uci/command_registry.hpp"
#include "util/exception.hpp"
#include "uci/uci_parser.hpp"
#include "uci/move_parsing.hpp"
#include "movegen/move_list.hpp"
#include <algorithm>

#ifdef LOKI_ENABLE_DEV_COMMANDS

using namespace loki;
using namespace loki::uci;

class move_command final : public uci_command<move_command>
{
public:
	move_command()
	{
		position_command = command_registry::instance().create("position");

		if (!position_command)
			throw_msg<loki_exception>("ERROR: position command needs to be registered for move command to work!");
	}

	static std::string name() { return "move"; }
	bool can_execute(const context* ctx) override { return ctx->state == UCI_STATE::Ready; }

	void execute(std::vector<std::string> arguments, context* ctx) override
	{
		if (arguments.empty())
			throw_msg<uci_parser::uci_error>("move cannot have zero arguments");

		const auto& gs = ctx->engine.position()->make_view()->game_state();
		auto move = parse_move_token(arguments[0], *gs);
		movegen::move_list moves;
		ctx->engine.position()->generate_moves(&moves);
		const auto pseudo_legal = std::ranges::find(moves, move);
		if (pseudo_legal == moves.end())
			throw_msg<uci_parser::uci_error>("move '{}' is illegal in the current position", move.to_string());

		if (!ctx->engine.position()->make_move(*pseudo_legal))
			throw_msg<uci_parser::uci_error>("move '{}' is illegal in the current position", move.to_string());
	}
	
private:
	std::unique_ptr<i_uci_command> position_command;
};

static command_registration<move_command> reg;

#endif
