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

using namespace loki::uci;

class stop_command final : public uci_command<stop_command>
{
public:
	static std::string name() { return "stop"; }
	bool can_execute(const context* ctx) override { return ctx->state == UCI_STATE::Searching; }

	void execute(std::vector<std::string>, context* ctx) override
	{
		ctx->engine.stop_search();
		ctx->state = UCI_STATE::Ready;
	}
};

static command_registration<stop_command> reg;
