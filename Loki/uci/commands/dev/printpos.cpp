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
#include "position/game_state.hpp"

#ifdef LOKI_ENABLE_DEV_COMMANDS

using namespace loki::uci;
using namespace loki;

class printpos_command final : public i_uci_command
{
public:
	std::string name() override { return "printpos"; }
	bool can_execute(const context* ctx) override { return ctx->state != UCI_STATE::Searching && ctx->state != UCI_STATE::Boot; }

	void execute(std::vector<std::string>, context* ctx) override
	{
		ctx->out << "POSITION: " << std::endl;
		const auto& game_state = ctx->engine.state();
		const auto& fen = position::game_state::to_fen(std::make_shared<position::game_state>(game_state));

		for (auto rank = position::RANK_8; rank >= position::RANK_1; rank--)
		{
			ctx->out << "\t" << rank + 1 << " ";

			for (auto file = position::FILE_A; file <= position::FILE_H; file++)
			{
				position::square sq(rank, file);
				side color;

				auto piece = game_state.get_piece(sq, &color);
				std::string piece_str = enum_to_string(piece);
				if (piece != NO_PIECE)
					piece_str = (color == WHITE) ? util::uppercase(piece_str) : util::lowercase(piece_str);
				ctx->out << piece_str << " ";
			}
			ctx->out << std::endl;
		}

		ctx->out << "\t" << " " << " "
			<< enum_to_string(position::FILE_A) << " "
			<< enum_to_string(position::FILE_B) << " "
			<< enum_to_string(position::FILE_C) << " "
			<< enum_to_string(position::FILE_D) << " "
			<< enum_to_string(position::FILE_E) << " "
			<< enum_to_string(position::FILE_F) << " "
			<< enum_to_string(position::FILE_G) << " "
			<< enum_to_string(position::FILE_H) << std::endl;
		ctx->out << "/POSITION" << std::endl << std::endl;

		ctx->out << "INFORMATION" << std::endl;
		ctx->out << "\tSide to move:\t\t" << enum_to_string(game_state.side_to_move) << std::endl;
		ctx->out << "\tEn-passant square:\t" << game_state.en_passant_sq.to_algebraic() << std::endl;;
		ctx->out << "\tCastling rights:\t" << game_state.castling_rights.to_string() << std::endl;
		ctx->out << "\tFifty move counter:\t" << game_state.fifty_move_cnt << std::endl;
		ctx->out << "\tFull move counter:\t" << game_state.full_move_cnt << std::endl;
		ctx->out << "\t" << "FEN:\t\t\t" << fen << std::endl;
		ctx->out << "/INFORMATION" << std::endl;
	}
};

static command_registration<printpos_command> reg;

#endif
