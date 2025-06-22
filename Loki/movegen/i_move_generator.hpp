#pragma once
#include "position/i_position_view.hpp"
#include "position/game_state.hpp"
#include "move_list.hpp"

namespace loki::movegen
{
	class i_move_generator
	{
		using pos_t = position::i_position_view;
		using ml_t = move_list;
	public:
		/// <summary>
		/// Generate all moves for the side to move.
		/// </summary>
		/// <typeparam name="MT">Move type to generate</typeparam>
		/// <typeparam name="PT">Piece type to generate for. To generate for all pieces, use the default NUM_PIECES</typeparam>
		/// <param name="pos">A pointer to a position object to generate for.</param>
		/// <param name="move_list">A move_list pointer to insert the result. Note that the original data will be cleared.</param>
		/// <returns>A size_t integer denoting the number of generated moves.</returns>
		template<move_type MT = ALL, piece PT = NUM_PIECES>
		inline size_t generate(const pos_t* pos, ml_t* ml) const
		{
			return generate_internal(pos, ml, pos->game_state()->side_to_move, MT, PT);
		}

		/// <summary>
		/// Generate all moves for a position given a piece type and side.
		/// </summary>
		/// <typeparam name="S">Side to generate for</typeparam>
		/// <typeparam name="MT">Move type to generate</typeparam>
		/// <typeparam name="PT">Piece type to generate for. To generate for all pieces, use the default NUM_PIECES</typeparam>
		/// <param name="pos">A pointer to a position object to generate for.</param>
		/// <param name="move_list">A move_list pointer to insert the result. Note that the original data will be cleared.</param>
		/// <returns>A size_t integer denoting the number of generated moves.</returns>
		template<side S, move_type MT = ALL, piece PT = NUM_PIECES> requires (PT <= NUM_PIECES) && (S < NUM_SIDES) && (MT <= PROMOTION)
			size_t generate(const pos_t* pos, ml_t* ml) const
		{
			return generate_internal(pos, ml, S, MT, PT);
		}

		/// <summary>
		/// Get the attackers of some piece type to a given square
		/// </summary>
		/// <typeparam name="PT">The piece type. Default: All pieces</typeparam>
		/// <param name="sq">The square</param>
		/// <param name="s">The side</param>
		/// <returns>A bitboard with the attacking pieces to the square</returns>
		template<piece PT = NUM_PIECES> requires (PT >= PAWN && PT <= NUM_PIECES)
			position::bitboard_t attackers_to(const pos_t* pos, position::e_square sq, side s) const
		{
			return attackers_to_internal(pos, sq, s, PT);
		}

		virtual ~i_move_generator() {}

	protected:
		virtual size_t generate_internal(const pos_t* pos, ml_t* ml, side s, move_type mt, piece pt) const = 0;
		virtual position::bitboard_t attackers_to_internal(const pos_t* pos, position::e_square sq, side s, piece pt) const = 0;
	};
}
