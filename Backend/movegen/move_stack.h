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
#ifndef MOVE_STACK_H
#define MOVE_STACK_H

namespace loki::movegen
{

	/// <summary>
	/// All info that is lost when a move is made.
	/// </summary>
	struct lost_move_info
	{
		PIECE		piece_captured;
		PIECE		piece_moved;
		uint8_t		castling_rights;
		size_t		fifty_moves_count;
		SQUARE		en_passant_square;
		bitboard_t	position_hash;

		void set(std::tuple<PIECE, PIECE, uint8_t, size_t, SQUARE, bitboard_t>&& info)
		{
			piece_captured = std::get<0>(info);
			piece_moved = std::get<1>(info);
			castling_rights = std::get<2>(info);
			fifty_moves_count = std::get<3>(info);
			en_passant_square = std::get<4>(info);
			position_hash = std::get<5>(info);
		}
	};

	/// <summary>
	/// move_stack has all the methods of fast_stack but a slightly optimized insert method for just this type of data.
	/// </summary>
	template<size_t _S> requires (_S > 0)
		class move_stack : public utility::fast_stack<std::pair<move_t, lost_move_info>, _S>
	{
		EXCEPTION_CLASS(e_moveStack, e_lokiError);
	public:
		/// <summary>
		/// Insert an entry into the stack.
		/// Note: Throws an exception if the container size limit is exceeded.
		/// </summary>
		/// <param name="move"></param>
		/// <param name="info"></param>
		inline void insert(move_t move, std::tuple<PIECE, PIECE, uint8_t, size_t, SQUARE, bitboard_t>&& info)
		{
			if (this->m_current_size >= this->max_size)
			{
				throw e_moveStack(FORMAT_EXCEPTION_MESSAGE("insert() called on a full stack. Limit exceeded."));
			}
			this->m_stack[this->m_current_size].first = move;
			this->m_stack[this->m_current_size].second.set(std::move(info));

			this->m_current_size++;
		}
	};
}

#endif
