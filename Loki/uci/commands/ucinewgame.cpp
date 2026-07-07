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
#include "position/game_state.hpp"
#include "position/io/game_state_builder.hpp"
#include "defs.hpp"
#include "util/exception.hpp"

#include <cassert>

using namespace loki::uci;
using namespace loki;

class ucinewgame_command final : public uci_command<ucinewgame_command>
{
public:
	static std::string name() { return "ucinewgame"; }
	bool can_execute(const context* ctx) override { return ctx->state == UCI_STATE::Boot || ctx->state == UCI_STATE::Ready; }

	void execute(std::vector<std::string>, context* ctx) override
	{
		assert(can_execute(ctx));

		auto gs = position::game_state::from_fen(constants::START_FEN);
		ctx->engine.clear();
		ctx->engine.set_position(*gs);

		ctx->state = UCI_STATE::Ready;
	}
};

static command_registration<ucinewgame_command> reg;
