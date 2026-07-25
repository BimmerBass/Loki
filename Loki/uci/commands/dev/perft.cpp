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

#ifdef LOKI_ENABLE_DEV_COMMANDS

using namespace loki;
using namespace loki::uci;

class perft_command final : public uci_command<perft_command>
{
public:
	static std::string name() { return "perft"; }
	bool can_execute(const context* ctx) override { return ctx->state == UCI_STATE::Ready; }

	void execute(std::vector<std::string> arguments, context* ctx) override
	{
		if (arguments.empty())
			throw_msg<uci_parser::uci_error>("perft needs at least one parameter");

		if (arguments.size() > 2 || (arguments.size() == 2 && arguments[1] != "hashvalidate"))
			throw_msg<uci_parser::uci_error>("optional perft parameter must be 'hashvalidate'");

		auto depth = std::stoi(arguments[0]);
		const bool hashvalidate = arguments.size() == 2;
		ctx->engine.perft(depth, ctx->out, hashvalidate);
	}
};

static command_registration<perft_command> reg;

#endif
