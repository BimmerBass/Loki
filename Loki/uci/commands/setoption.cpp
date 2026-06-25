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

class setoption_command final : public uci_command<setoption_command>
{
public:
	static std::string name()
	{
		return "setoption";
	}

	bool can_execute(const context* /**/) override
	{
		loki::throw_msg<loki::not_implemented_error>("not implemented");
	}

	void execute(std::vector<std::string>, context* /**/) override
	{
		loki::throw_msg<loki::not_implemented_error>("not implemented");
	}
};

static command_registration<setoption_command> reg;
