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
#include "uci/move_parsing.hpp"
#include "movegen/move.hpp"
#include "position/game_state.hpp"
#include "position/square.hpp"
#include "util/exception.hpp"
#include "position/io/game_state_builder.hpp"
#include "defs.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <iterator>
#include <vector>

using namespace loki::uci;
using namespace loki;


class position_command final : public uci_command<position_command>
{
public:
	static std::string name()
	{
		return "position";
	}

	bool can_execute(const context* ctx) override
	{
		switch (ctx->state)
		{
		case UCI_STATE::Boot:
		case UCI_STATE::Ready:
			return true;
		default:
			return false;
		}
	}

	void execute(std::vector<std::string> tokens, context* ctx) override
	{
		assert(can_execute(ctx));
		std::string fen;

		if (tokens.empty())
			throw_msg<uci_parser::uci_error>("position needs at least one parameter.");

		// position
		auto it = tokens.begin();
		if (*it == "fen")
		{
			++it;
			if (it == tokens.end())
				throw_msg<uci_parser::uci_error>("position needs a FEN string after 'fen'.");

			fen = *it;
			while (++it != tokens.end() && *it != "moves")
				fen += " " + *it;
		}
		else if (*it == "startpos")
		{
			++it;
			fen = constants::START_FEN;
		}
		else
		{
			throw_msg<uci_parser::uci_error>("invalid first parameter for position command: '{}'", *it);
		}
	
		try
		{
			const auto gs = position::game_state::from_fen(fen);
			auto new_pos = ctx->engine.make_position(*gs);

			// move parsing
			if (const auto moves_it = std::find(tokens.begin(), tokens.end(), "moves"); moves_it != tokens.end())
			{
				const auto view = new_pos->make_view();
				for (auto move_it = std::next(moves_it); move_it != tokens.end(); ++move_it)
				{
					const auto state = view.game_state();
					auto move = parse_move_token(*move_it, *state);
					if (!new_pos->make_move(move))
						throw uci_parser::uci_error("move sequence contains one or more invalid moves");
				}
			}

			ctx->engine.set_position(std::move(new_pos));
		}
		catch (const position::io::game_state_builder::fen_parsing_error& e)
		{
			throw_msg<uci_parser::uci_error>("invalid FEN: {}", e.what());
		}

		ctx->state = UCI_STATE::Ready;
	}
};

static command_registration<position_command> reg;
