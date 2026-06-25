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
#include "movegen/move_list.hpp"

#ifdef LOKI_ENABLE_DEV_COMMANDS

using namespace loki;
using namespace loki::uci;
using namespace loki::movegen;

class printmoves_command final : public uci_command<printmoves_command>
{
public:
	static std::string name() { return "printmoves"; }
	bool can_execute(const context* ctx) override { return ctx->state == UCI_STATE::Ready; }

	void execute(std::vector<std::string> parameters, context* ctx) override
	{
		const auto& pos = ctx->engine.position();
		move_list ml;

		auto type = parameters.empty() ? "all" : util::lowercase(parameters[0]);

		switch (hash(type))
		{
		case "all"_hash:
			pos->generate_moves<movegen::ALL>(&ml);
			break;
		case "quiet"_hash:
			pos->generate_moves<movegen::QUIET>(&ml);
			break;
		case "active"_hash:
			pos->generate_moves<movegen::ACTIVE>(&ml);
			break;
		default:
			throw_msg<uci_parser::uci_error>("invalid argument to printmoves: '{}'", type);
		}

		ctx->out << type << " moves for position:" << std::endl;

		auto i = 1;
		for (const auto& m : ml)
		{
			ctx->out << std::format("[{}] {}", i++, m.to_string()) << std::endl;
		}
	}
};

static command_registration<printmoves_command> reg;

#endif
