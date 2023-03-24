//
//	Loki, a UCI-compliant chess playing software
//	Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
//	Loki is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	Loki is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
#include "loki.pch.hpp"
#include <map>

namespace loki::position
{

#pragma region FEN-Parsing helpers
	namespace
	{
		EXCEPTION_CLASS(e_invalidFen, game_state::e_gameState);

		std::vector<std::stringstream> split_string(std::stringstream& ss, char delim = ' ')
		{
			std::vector<std::stringstream> output;
			std::string token;

			while (std::getline(ss, token, delim))
			{
				output.push_back(std::stringstream(token));
			}
			return output;
		}

		namespace write
		{
			void position(const game_state* pos, std::stringstream& ss)
			{
				// Firstly, create an array for all piece placements.
				char piece_list[6] = { 'p', 'n', 'b', 'r', 'q', 'k' };
				std::array<char, SQ_NB> piece_array;
				piece_array.fill('0');

				for (size_t pce = PAWN; pce <= KING; pce++)
				{
					bitboard_t whites = pos->piece_placements[WHITE][pce];
					bitboard_t blacks = pos->piece_placements[BLACK][pce];

					while (whites)
					{
						auto inx = pop_bit(whites);
						piece_array[inx] = std::toupper(piece_list[pce]);
					}
					while (blacks)
					{
						auto inx = pop_bit(blacks);
						piece_array[inx] = piece_list[pce];
					}
				}

				std::array<std::string, RANK_NB> rank_piece_placements;
				for (size_t r = RANK_1; r <= RANK_8; r++)
				{
					size_t fen_rank = RANK_8 - r;
					size_t skipped_files = 0;

					for (size_t f = FILE_A; f <= FILE_H; f++)
					{
						if (piece_array[get_square(r, f)] == '0')
						{
							skipped_files++;

							if (f == FILE_H)
							{
								rank_piece_placements[fen_rank] += std::to_string(skipped_files);
							}
							continue;
						}
						if (skipped_files)
						{
							rank_piece_placements[fen_rank] += std::to_string(skipped_files);
						}
						rank_piece_placements[fen_rank] += piece_array[get_square(r, f)];
						skipped_files = 0;
					}
				}

				for (size_t i = 0; i < RANK_NB; i++)
				{
					ss << (i == 0 ? "" : "/") << rank_piece_placements[i];
				}
			}
			std::string castling(const game_state* pos)
			{
				if (!pos->castling_rights.any())
				{
					return "-";
				}

				return (pos->castling_rights.operator() < WKCA > () ? "K" : "") +
					std::string((pos->castling_rights.operator() < WQCA > () ? "Q" : "")) +
					std::string((pos->castling_rights.operator() < BKCA > () ? "k" : "")) +
					std::string((pos->castling_rights.operator() < BQCA > () ? "q" : ""));
			}
			std::string en_passant(const game_state* pos)
			{
				if (pos->en_passant_square == NO_SQ)
				{
					return "-";
				}
				return to_algebraic(pos->en_passant_square);
			}
		}
		namespace read
		{
			void clear_gamestate(game_state* pos)
			{
				for (size_t pce = PAWN; pce <= KING; pce++)
				{
					pos->piece_placements[WHITE][pce] = 0;
					pos->piece_placements[BLACK][pce] = 0;
				}
				pos->side_to_move = WHITE;
				pos->fifty_move_counter = 0;
				pos->full_move_counter = 0;
				pos->en_passant_square = NO_SQ;
				pos->castling_rights.clear();
			}

			void position(game_state* pos, std::stringstream& ss)
			{
				auto ranks = split_string(ss, '/');

				if (ranks.size() != RANK_NB)
				{
					throw e_invalidFen(FORMAT_EXCEPTION_MESSAGE("Invalid FEN: There should be exactly eight fields separated by '/' to describe the piece placements."));
				}

				for (auto r = 0; r < ranks.size(); r++)
				{
					auto rank_pieces = ranks[r].str();

					size_t internal_rank = RANK_8 - r; // Since the position description starts at the eighth rank, we need to invert this.
					size_t current_token_inx = 0;
					size_t skipped_files = 0;

					for (size_t f = FILE_A; f <= FILE_H; f++)
					{
						if (std::isdigit(rank_pieces[current_token_inx]))
						{
							f += (static_cast<size_t>(rank_pieces[current_token_inx] - '0') - 1);
						}
						else
						{
							switch (rank_pieces[current_token_inx])
							{
							case 'P': pos->piece_placements[WHITE][PAWN] |= (bitboard_t(1) << get_square(internal_rank, f)); break;
							case 'N': pos->piece_placements[WHITE][KNIGHT] |= (bitboard_t(1) << get_square(internal_rank, f)); break;
							case 'B': pos->piece_placements[WHITE][BISHOP] |= (bitboard_t(1) << get_square(internal_rank, f)); break;
							case 'R': pos->piece_placements[WHITE][ROOK] |= (bitboard_t(1) << get_square(internal_rank, f)); break;
							case 'Q': pos->piece_placements[WHITE][QUEEN] |= (bitboard_t(1) << get_square(internal_rank, f)); break;
							case 'K': pos->piece_placements[WHITE][KING] |= (bitboard_t(1) << get_square(internal_rank, f)); break;
							case 'p': pos->piece_placements[BLACK][PAWN] |= (bitboard_t(1) << get_square(internal_rank, f)); break;
							case 'n': pos->piece_placements[BLACK][KNIGHT] |= (bitboard_t(1) << get_square(internal_rank, f)); break;
							case 'b': pos->piece_placements[BLACK][BISHOP] |= (bitboard_t(1) << get_square(internal_rank, f)); break;
							case 'r': pos->piece_placements[BLACK][ROOK] |= (bitboard_t(1) << get_square(internal_rank, f)); break;
							case 'q': pos->piece_placements[BLACK][QUEEN] |= (bitboard_t(1) << get_square(internal_rank, f)); break;
							case 'k': pos->piece_placements[BLACK][KING] |= (bitboard_t(1) << get_square(internal_rank, f)); break;
							default:
								throw e_invalidFen(FORMAT_EXCEPTION_MESSAGE("Invalid FEN: Illegal character found while parsing piece placements."));
							}
						}
						current_token_inx++;
					}
				}
			}
			void side_to_move(game_state* pos, std::stringstream& ss)
			{
				if (ss.str().size() != 1 || (std::tolower(ss.str()[0]) != 'w' && std::tolower(ss.str()[0]) != 'b'))
					throw e_invalidFen(FORMAT_EXCEPTION_MESSAGE("Invalid FEN: Side to move is ill-formed"));
				pos->side_to_move = std::tolower(ss.str()[0]) == 'w' ? WHITE : BLACK;
			}
			void castling(game_state* pos, std::stringstream& ss)
			{
				auto castling_str = ss.str();

				if (castling_str.find_first_not_of("KQkq-") != std::string::npos)
					throw e_invalidFen(FORMAT_EXCEPTION_MESSAGE("Invalid FEN: Castling rights are ill-formed"));

				if (castling_str == "-")
				{
					pos->castling_rights.clear();
				}
				else
				{
					if (castling_str.find('K') != std::string::npos)
					{
						pos->castling_rights += WKCA;
					}
					if (castling_str.find('Q') != std::string::npos)
					{
						pos->castling_rights += WQCA;
					}
					if (castling_str.find('k') != std::string::npos)
					{
						pos->castling_rights += BKCA;
					}
					if (castling_str.find('q') != std::string::npos)
					{
						pos->castling_rights += BQCA;
					}
				}
			}
			void en_passant(game_state* pos, std::stringstream& ss)
			{
				if (ss.str() == "-")
				{
					pos->en_passant_square = NO_SQ;
					return;
				}
				if (!is_algebraic(ss.str()))
					throw e_invalidFen(FORMAT_EXCEPTION_MESSAGE("Invalid FEN: En-passant square was not null, while also not being in algebraic notation."));
				pos->en_passant_square = from_algebraic(ss.str());
			}
			void move_clocks(game_state* pos, std::stringstream& half_ss, std::stringstream& full_ss)
			{
				try
				{
					pos->fifty_move_counter = std::stoi(half_ss.str());
					pos->full_move_counter = std::stoi(full_ss.str());
				}
				catch (const std::invalid_argument& e)
				{
					throw e_invalidFen(FORMAT_EXCEPTION_MESSAGE("Invalid FEN: Error parsing move clocks (std::invalid_argument: )" + std::string(e.what())));
				}
				catch (const std::out_of_range& e)
				{
					throw e_invalidFen(FORMAT_EXCEPTION_MESSAGE("Invalid FEN: Error parsing move clocks (std::out_of_range: )" + std::string(e.what())));
				}
			}
		}
	}
#pragma endregion

	void game_state::operator<<(const std::string& fen)
	{
		if (fen.empty())
			throw e_invalidFen(FORMAT_EXCEPTION_MESSAGE("FEN was an empty string"));
		read::clear_gamestate(this);

		std::stringstream ss(fen);
		auto fen_fields = split_string(ss);
		if (fen_fields.size() < 3)
			throw e_invalidFen(FORMAT_EXCEPTION_MESSAGE("piece placement, side to move and castling rights are strictly required according to the Forsyth-Edwards Notation."));

		// There will always be at least one field
		read::position(this, fen_fields[0]);
		read::side_to_move(this, fen_fields[1]);
		read::castling(this, fen_fields[2]);

		// En-passant square and move clocks aren't strictly needed in order for a position to be valid, so Loki doesn't require them.
		if (fen_fields.size() >= 4) read::en_passant(this, fen_fields[3]);
		if (fen_fields.size() >= 6)
		{
			read::move_clocks(this, fen_fields[4], fen_fields[5]);
		}
	}

	void game_state::operator>>(std::string& fen) const
	{

		std::stringstream fen_ss("");
		write::position(this, fen_ss);

		fen_ss << std::format(" {} {} {}",
			side_to_move == WHITE ? "w" : "b",
			write::castling(this),
			write::en_passant(this));
		if (fifty_move_counter > 0 || full_move_counter > 0)
		{
			fen_ss << " " << fifty_move_counter;
			fen_ss << " " << full_move_counter;
		}

		fen = fen_ss.str();
	}


	std::ostream& operator<<(std::ostream& os, const game_state& gs)
	{
		const static std::array<char, PIECE_NB> piece_letters = { 'p', 'n', 'b', 'r', 'q', 'k' };
		std::string output = "................................................................";

		for (size_t pce = PAWN; pce <= KING; pce++)
		{
			bitboard_t whites = gs.piece_placements[WHITE][pce];
			bitboard_t blacks = gs.piece_placements[BLACK][pce];

			while (whites)
			{
				auto inx = pop_bit(whites);
				output[inx] = std::toupper(piece_letters[pce]);
			}
			while (blacks)
			{
				auto inx = pop_bit(blacks);
				output[inx] = piece_letters[pce];
			}
		}

		os << "\nGame state:\n\n";
		for (int r = RANK_8; r >= RANK_1; r--)
		{
			os << "\t" << r + 1 << "  ";
			for (size_t f = FILE_A; f <= FILE_H; f++)
			{
				os << output[get_square(r, f)] << " ";
			}
			os << "\n";
		}
		os << "\n";
		os << "\t  a b c d e f g h\n\n";
		os << "\tSide to move: " << (gs.side_to_move == WHITE ? "White" : "Black") << "\n";
		os << "\tEn passant: " << (gs.en_passant_square == NO_SQ ? "N/A" : to_algebraic(gs.en_passant_square)) << "\n";
		os << "\tCastling rights: " << write::castling(&gs) << "\n";
		os << "\tFifty moves counter: " << gs.fifty_move_counter << "\n";
		os << "\tFull move clock: " << gs.full_move_counter << "\n";

		return os;
	}
}