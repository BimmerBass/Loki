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

namespace
{
	using loki::movegen::move;
	using loki::movegen::move_attr;

	move_attr infer_move_attr(const position::game_state& state, const move& candidate)
	{
		side mover = NUM_SIDES;
		const auto moved_piece = state.get_piece(position::square(candidate.from()), &mover);
		if (moved_piece == NO_PIECE || mover == NUM_SIDES)
			throw_msg<uci_parser::uci_error>("invalid move: no piece exists on '{}'", position::square(candidate.from()).to_algebraic());

		if (candidate.type() == loki::movegen::PROMOTION)
			return loki::movegen::PROMOTION;

		if (moved_piece == PAWN && state.en_passant_sq == candidate.to())
		{
			const auto dest_piece = state.get_piece(position::square(candidate.to()));
			if (dest_piece == NO_PIECE)
				return loki::movegen::ENPASSANT;
		}

		if (moved_piece == KING
			&& position::rank_of(candidate.from()) == position::rank_of(candidate.to())
			&& std::abs(static_cast<int>(position::file_of(candidate.to())) - static_cast<int>(position::file_of(candidate.from()))) == 2)
			return loki::movegen::CASTLING;

		return loki::movegen::NORMAL;
	}

	move parse_move_token(const std::string& token, const position::game_state& state)
	{
		if (token.size() < 4 || token.size() > 5)
			throw_msg<uci_parser::uci_error>("invalid move token: '{}'", token);

		const position::square from_sq(token.substr(0, 2));
		const position::square to_sq(token.substr(2, 2));

		auto type = loki::movegen::NORMAL;
		auto promotion_piece = KNIGHT;
		if (token.size() == 5)
		{
			type = loki::movegen::PROMOTION;
			switch (std::tolower(static_cast<unsigned char>(token[4])))
			{
			case 'n': promotion_piece = KNIGHT; break;
			case 'b': promotion_piece = BISHOP; break;
			case 'r': promotion_piece = ROOK; break;
			case 'q': promotion_piece = QUEEN; break;
			default:
				throw_msg<uci_parser::uci_error>("invalid promotion piece in move token: '{}'", token);
			}
		}

		move candidate(from_sq.value(), to_sq.value(), type, promotion_piece);
		candidate.type(infer_move_attr(state, candidate));
		return candidate;
	}
}

class position_command final : public i_uci_command
{
public:
	std::string name() override
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
		std::vector<move> moves;
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

			// move parsing
			if (const auto moves_it = std::find(tokens.begin(), tokens.end(), "moves"); moves_it != tokens.end())
			{
				for (auto move_it = std::next(moves_it); move_it != tokens.end(); ++move_it)
					moves.push_back(parse_move_token(*move_it, *gs));
			}

			if (!ctx->engine.set_position(*gs, moves))
			{
				throw uci_parser::uci_error("move sequence contains one or more invalid moves");
			}
		}
		catch (const position::io::game_state_builder::fen_parsing_error& e)
		{
			throw_msg<uci_parser::uci_error>("invalid FEN: {}", e.what());
		}

		ctx->state = UCI_STATE::Ready;
	}
};

static command_registration<position_command> reg;
