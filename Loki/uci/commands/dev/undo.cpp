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
#include "uci/uci_parser.hpp"

#ifdef LOKI_ENABLE_DEV_COMMANDS

using namespace loki;
using namespace loki::uci;

class undo : public uci_command<undo>
{
public:
	static std::string name() { return "undo"; }
	bool can_execute(const context* ctx) override { return ctx->state == UCI_STATE::Ready; }

	void execute(std::vector<std::string>, context* ctx) override
	{
		if (!ctx->engine.position()->has_made_move())
			throw_msg<uci_parser::uci_error>("no move has been made in the current position object");
		ctx->engine.position()->undo_last_move();
	}
};

static command_registration<undo> reg;

#endif