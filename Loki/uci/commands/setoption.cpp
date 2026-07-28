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

using namespace loki;
using namespace loki::uci;

class setoption_command final : public uci_command<setoption_command>
{
public:
	static std::string name() { return "setoption"; }
	bool can_execute(const context* ctx) override { return ctx->state == UCI_STATE::Boot || ctx->state == UCI_STATE::Ready; }

	void execute(std::vector<std::string> params, context* ctx) override
	{
		auto it = params.begin();
		if (*(it++) != "name" || it == params.end())
			throw_msg<uci_parser::uci_error>("first parameter was not 'name' or had no value associated.");

		auto value_it = std::ranges::find(params, "value");
		auto name = fold_left(std::ranges::subrange(it, value_it));
		if (name.empty())
			throw_msg<uci_parser::uci_error>("no name was provided");

		std::optional<std::string> value = fold_left(std::ranges::subrange(++value_it, params.end()));
		if (value->empty())
			value = std::nullopt;

		try
		{
			ctx->engine.options().apply(name, value);
		}
		catch (const options::registry::option_error& e)
		{
			throw_msg<uci_parser::uci_error>("{}", e.what());
		}
	}

private:
	using it_t = std::vector<std::string>::iterator;
	inline static std::string fold_left(std::ranges::subrange<it_t> srng)
	{
		return std::ranges::fold_left(srng, std::string{},
			[](std::string result, const std::string& value)
			{
				if (!result.empty())
					result += " ";
				result += value;
				return result;
			});
	}
};

static command_registration<setoption_command> reg;
