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
#include "search_position.hpp"
#include "movegen/move_generator.hpp"
#include "square.hpp"
#include <algorithm>
#include <numeric>

namespace loki::position
{
	using namespace movegen;

	search_position_t make(game_state_t state, magics::magic_index_t bishop_index, magics::magic_index_t rook_index)
	{
		auto ptr = new search_position(
			std::make_unique<game_state>(*state),
			std::unique_ptr<i_move_generator<search_position::position_proxy>>(new move_generator<search_position::position_proxy>(rook_index, bishop_index)));
		return search_position_t(ptr);
	}

	search_position::search_position(std::unique_ptr<game_state> state, std::unique_ptr<movegen::i_move_generator<position_proxy>> move_generator)
		: m_state{ std::move(state) }, 
		m_history{},
		m_ply{ ROOT_PLY },
		m_move_generator{ std::move(move_generator) }
	{
		// Fill m_piecebbs
		for (auto side = WHITE; side <= BLACK; side++)
		{
			auto& piecebbs = m_piecebbs[side];
			auto& piece_placements = m_state->piece_placements[side];
			std::fill(piecebbs, piecebbs + std::size(piecebbs), 0ULL);
			for (auto sq = A1; sq <= H8; sq++)
			{
				switch (piece_placements[sq])
				{
				case PAWN: piecebbs[PAWN] = set_one_at(piecebbs[PAWN], sq); break;
				case KNIGHT: piecebbs[KNIGHT] = set_one_at(piecebbs[KNIGHT], sq); break;
				case BISHOP: piecebbs[BISHOP] = set_one_at(piecebbs[BISHOP], sq); break;
				case ROOK: piecebbs[ROOK] = set_one_at(piecebbs[ROOK], sq); break;
				case QUEEN: piecebbs[QUEEN] = set_one_at(piecebbs[QUEEN], sq); break;
				case KING: piecebbs[KING] = set_one_at(piecebbs[KING], sq); break;
				default:
					continue;
				}
			}

			// Fill all pieces for the side
			m_all_pieces[side] = std::accumulate(
				piecebbs, piecebbs + std::size(piecebbs), 0ULL,
				[](bitboard_t acc, uint64_t bb) { return acc | bb; });
		}

		// Fill m_king_squares
		m_king_squares[WHITE] = static_cast<e_square>(scan_lsb(m_piecebbs[WHITE][KING]).value());
		m_king_squares[BLACK] = static_cast<e_square>(scan_lsb(m_piecebbs[BLACK][KING]).value());
	}

	std::unique_ptr<search_position> search_position::clone() const
	{
		auto mg = new move_generator<position_proxy>(m_move_generator->rook_index(), m_move_generator->bishop_index());
		auto ptr = new search_position(
			std::make_unique<game_state>(*m_state),
			std::unique_ptr<i_move_generator<position_proxy>>(mg));
		ptr->m_history = m_history;
		ptr->m_ply = m_ply;
		
		return std::unique_ptr<search_position>(ptr);
	}

	bool search_position::make_move(const move& move)
	{
		auto me = m_state->side_to_move;
		auto piece_moved = m_state->piece_placements[me][move.from()];
		auto piece_captured = m_state->piece_placements[!me][move.to()];
		assert(piece_moved >= PAWN && piece_moved <= KING);
		
		if (piece_captured == KING)
			return false;

		// Save data than can't be inferred from the resulting position
		m_history.push(
			move.get_move(),
			piece_captured, piece_moved,
			m_state->castling_rights.get(),
			m_state->fifty_move_cnt,
			m_state->en_passant_sq);
		m_ply += 1;

		// remove from origin
		m_piecebbs[me][piece_moved] = toggle_at(m_piecebbs[me][piece_moved], move.from());
		m_state->piece_placements[me][move.from()] = NO_PIECE;
		
		// add to destination
		if (move.type() == PROMOTION)
		{
			assert(!is_one_at(m_piecebbs[me][move.promotion_piece()], move.to()));
			m_piecebbs[me][move.promotion_piece()] = toggle_at(m_piecebbs[me][move.promotion_piece()], move.to());
			m_state->piece_placements[me][move.to()] = move.promotion_piece();
		}
		else
		{
			if (move.type() == CASTLING)
			{
				bool is_kingside = move.to() > move.from();
				auto rank = me == WHITE ? RANK_1 : RANK_8;
				square rk_from(rank, is_kingside ? FILE_H : FILE_A), rk_to(rank, is_kingside ? FILE_F : FILE_D);

				// remove rook from corner
				m_piecebbs[me][ROOK] = toggle_at(m_piecebbs[me][ROOK], rk_from.value());
				m_state->piece_placements[me][rk_from.value()] = NO_PIECE;

				// add rook
				m_piecebbs[me][ROOK] = toggle_at(m_piecebbs[me][ROOK], rk_to.value());
				m_state->piece_placements[me][rk_to.value()] = ROOK;
			}
			else if (move.type() == ENPASSANT)
			{
				// remove the pawn.
				auto cap_sq = me == WHITE ? m_state->en_passant_sq.value() - 8 : m_state->en_passant_sq.value() + 8;
				m_piecebbs[!me][PAWN] = toggle_at(m_piecebbs[!me][PAWN], cap_sq);
				m_state->piece_placements[!me][cap_sq] = NO_PIECE;
			}

			// add moved piece to destination
			m_piecebbs[me][piece_moved] = toggle_at(m_piecebbs[me][piece_moved], move.to());
			m_state->piece_placements[me][move.to()] = piece_moved;

			if (piece_moved == KING)
				m_king_squares[me] = move.to();
		}

		// remove the potentially captured piece
		if (piece_captured != NO_PIECE)
		{
			m_state->piece_placements[!me][move.to()] = NO_PIECE;
			m_piecebbs[!me][piece_captured] = toggle_at(m_piecebbs[!me][piece_captured], move.to());
		}

		// update castling rights
		if (piece_moved == KING)
		{
			m_state->castling_rights.set<KINGSIDE>(me, false);
			m_state->castling_rights.set<QUEENSIDE>(me, false);
		}
		// if a piece has been moved to or from a corner square, that square's castling rights will be removed
		if (move.to() == A1 || move.from() == A1)
			m_state->castling_rights.set<WHITE, QUEENSIDE>(false);
		if (move.to() == H1 || move.from() == H1)
			m_state->castling_rights.set<WHITE, KINGSIDE>(false);
		if (move.to() == A8 || move.from() == A8)
			m_state->castling_rights.set<BLACK, QUEENSIDE>(false);
		if (move.to() == H8 || move.from() == H8)
			m_state->castling_rights.set<BLACK, KINGSIDE>(false);

		// update en passant
		m_state->en_passant_sq = NO_SQ;
		auto from_rank = static_cast<int>(rank_of(move.from()));
		auto to_rank = static_cast<int>(rank_of(move.to()));
		if (piece_moved == PAWN && abs(to_rank - from_rank) == 2)
			m_state->en_passant_sq = me == WHITE ? move.to() - 8 : move.to() + 8;

		// update all_pieces bitboards
		// Manually OR each piece bitboard for each side. This avoids the overhead of std::accumulate
		// and the lambda call per element which was visible in profiling.
		m_all_pieces[WHITE] =
			m_piecebbs[WHITE][PAWN] |
			m_piecebbs[WHITE][KNIGHT] |
			m_piecebbs[WHITE][BISHOP] |
			m_piecebbs[WHITE][ROOK] |
			m_piecebbs[WHITE][QUEEN] |
			m_piecebbs[WHITE][KING];
		m_all_pieces[BLACK] =
			m_piecebbs[BLACK][PAWN] |
			m_piecebbs[BLACK][KNIGHT] |
			m_piecebbs[BLACK][BISHOP] |
			m_piecebbs[BLACK][ROOK] |
			m_piecebbs[BLACK][QUEEN] |
			m_piecebbs[BLACK][KING];

		// move counter updates
		m_state->fifty_move_cnt++;
		if (me == BLACK)
			m_state->full_move_cnt++;
		
		// If the move is a capture or a pawn move, the fifty move counter is reset to zero.
		if (piece_moved == PAWN || piece_captured != NO_PIECE)
			m_state->fifty_move_cnt = 0;

		// undo the move if we're in check since its illegal
		if (in_check())
		{
			m_state->side_to_move = !me; // undo_move toggles this
			undo_last_move();
			return false;
		}
		m_state->side_to_move = !me;
		return true;
	}

	void search_position::undo_last_move()
	{
		const auto& [raw_move, metadata] = m_history.pop();
		if (m_ply > ROOT_PLY)
			m_ply -= 1;
		move move = raw_move;
		
		// toggle side to move
		m_state->side_to_move = !m_state->side_to_move;
		side me = m_state->side_to_move;

		// cache squares and masks to avoid repeated calls
		auto from_sq = move.from();
		auto to_sq = move.to();
		const bitboard_t from_mask = bitboard_t(1) << from_sq;
		const bitboard_t to_mask = bitboard_t(1) << to_sq;

		// place piece moved at origin and remove from destination.
		auto dest_piece = move.type() == PROMOTION ? move.promotion_piece() : metadata.moved;
		m_state->piece_placements[me][to_sq] = NO_PIECE;
		m_piecebbs[me][dest_piece] = toggle_at(m_piecebbs[me][dest_piece], to_sq);
		// toggle overall occupancy for the side at destination
		m_all_pieces[me] ^= to_mask;

		m_state->piece_placements[me][from_sq] = metadata.moved;
		m_piecebbs[me][metadata.moved] = toggle_at(m_piecebbs[me][metadata.moved], from_sq);
		// toggle overall occupancy for the side at origin
		m_all_pieces[me] ^= from_mask;

		if (metadata.moved == KING)
			m_king_squares[me] = from_sq;

		// handle special moves
		e_square pawn_sq;
		bool is_kingside;
		e_rank rank;
		square rk_from, rk_to;
		switch (move.type())
		{
		case ENPASSANT:
			pawn_sq = me == WHITE ? metadata.ep_sq.value() - 8 : metadata.ep_sq.value() + 8;
			m_state->piece_placements[!me][pawn_sq] = PAWN;
			m_piecebbs[!me][PAWN] = toggle_at(m_piecebbs[!me][PAWN], pawn_sq);
			m_all_pieces[!me] ^= (bitboard_t(1) << pawn_sq);
			break;
		case CASTLING:
			is_kingside = move.to() > move.from();
			rank = me == WHITE ? RANK_1 : RANK_8;
			rk_from = square(rank, is_kingside ? FILE_H : FILE_A);
			rk_to = square(rank, is_kingside ? FILE_F : FILE_D);

			// remove rook from dest.
			m_state->piece_placements[me][rk_to.value()] = NO_PIECE;
			m_piecebbs[me][ROOK] = toggle_at(m_piecebbs[me][ROOK], rk_to.value());
			m_all_pieces[me] ^= (bitboard_t(1) << rk_to.value());

			// add rook to corner
			m_state->piece_placements[me][rk_from.value()] = ROOK;
			m_piecebbs[me][ROOK] = toggle_at(m_piecebbs[me][ROOK], rk_from.value());
			m_all_pieces[me] ^= (bitboard_t(1) << rk_from.value());
			break;
		default: break;
		}

		// handle captured piece
		if (metadata.captured != NO_PIECE)
		{
			m_state->piece_placements[!me][to_sq] = metadata.captured;
			m_piecebbs[!me][metadata.captured] = toggle_at(m_piecebbs[!me][metadata.captured], to_sq);
			m_all_pieces[!me] ^= to_mask;
		}

		// update lost info (m_all_pieces updated incrementally above)
		m_state->castling_rights = metadata.castle_rights;
		m_state->fifty_move_cnt = metadata.fifty_move;
		m_state->full_move_cnt -= me == BLACK ? 1 : 0;
		m_state->en_passant_sq = metadata.ep_sq;
	}

	bool search_position::in_check() const
	{
		position_proxy proxy = this;
		return m_move_generator->attackers_to(&proxy, m_king_squares[m_state->side_to_move], !m_state->side_to_move);
	}

	bool search_position::is_draw() const noexcept
	{
		return m_state->fifty_move_cnt >= 100 || is_material_draw() || is_repetition();
	}

	bool search_position::is_material_draw() const noexcept
	{
		// two kings -> immediate draw
		const auto kings_bb = m_piecebbs[WHITE][KING] | m_piecebbs[BLACK][KING];
		const auto total_occupancy = m_all_pieces[WHITE] | m_all_pieces[BLACK];
		if (kings_bb == total_occupancy)
			return true;

		const auto num_pieces = popcount(total_occupancy);

		// king against single minor and king
		const auto all_minors =
			m_piecebbs[WHITE][KNIGHT] | m_piecebbs[WHITE][BISHOP] |
			m_piecebbs[BLACK][KNIGHT] | m_piecebbs[BLACK][BISHOP];
		if (num_pieces == 3 && all_minors != 0)
			return true;

		// king and bishop vs king and bishop is a draw if the bishops are on the same color.
		const auto white_bishops = popcount(m_piecebbs[WHITE][BISHOP]);
		const auto black_bishops = popcount(m_piecebbs[BLACK][BISHOP]);
		const auto white_bishop_dark_square = (m_piecebbs[WHITE][BISHOP] | DARK_SQUARES) == DARK_SQUARES;
		const auto black_bishop_dark_square = (m_piecebbs[BLACK][BISHOP] | DARK_SQUARES) == DARK_SQUARES;

		if (num_pieces == 4 && white_bishops == 1 && black_bishops == 1 && white_bishop_dark_square == black_bishop_dark_square)
			return true;

		return false;
	}

	bool search_position::is_repetition() const noexcept
	{
		return false;
	}

	template<move_type MT>
	[[maybe_unused]] size_t search_position::generate_moves(movegen::move_list* ml) const
	{
		position_proxy proxy = this;
		return m_move_generator->generate<MT>(&proxy, ml);
	}

	[[maybe_unused]] size_t search_position::generate_all_legals(movegen::move_list* ml)
	{
		ml->clear();
		movegen::move_list pseudolegals;
		generate_moves(&pseudolegals);

		for (const auto& move : pseudolegals)
		{
			if (!make_move(move))
				continue;
			ml->push_back(move);
			undo_last_move();
		}
		return ml->size();
	}

	template size_t search_position::generate_moves<movegen::ALL>(movegen::move_list*) const;
	template size_t search_position::generate_moves<movegen::ACTIVE>(movegen::move_list*) const;
	template size_t search_position::generate_moves<movegen::QUIET>(movegen::move_list*) const;
}
