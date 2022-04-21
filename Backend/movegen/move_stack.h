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

namespace loki::movegen {

	/// <summary>
	/// All info that is lost when a move is made.
	/// </summary>
	struct lost_move_info {
		PIECE		piece_captured;
		PIECE		piece_moved;
		uint8_t		castling_rights;
		size_t		fifty_moves_count;
		SQUARE		en_passant_square;
		bitboard_t	position_hash;

		void set(std::tuple<PIECE, PIECE, uint8_t, size_t, SQUARE, bitboard_t>&& info) {
			piece_captured		= std::get<0>(info);
			piece_moved			= std::get<1>(info);
			castling_rights		= std::get<2>(info);
			fifty_moves_count	= std::get<3>(info);
			en_passant_square	= std::get<4>(info);
			position_hash		= std::get<5>(info);
		}
	};

	/// <summary>
	/// A simple implementation of a stack-allocated stack.
	/// </summary>
	template<size_t _Size>
	class move_stack {
	public:
		using value_t = std::pair<move_t, lost_move_info>;
	private:
		value_t m_stack[_Size] = {};
		size_t m_current_size = 0;

	public:
		/// <summary>
		/// Insert an entry into the stack.
		/// Note: Throws std::out_of_range if the container size limit is exceeded.
		/// </summary>
		/// <param name="move"></param>
		/// <param name="info"></param>
		void insert(move_t move, std::tuple<PIECE, PIECE, uint8_t, size_t, SQUARE, bitboard_t>&& info) {
			if (m_current_size >= _Size) {
				throw std::out_of_range("insert() called on a full stack. Limit exceeded.");
			}
			m_stack[m_current_size].first = move;
			m_stack[m_current_size].second.set(std::move(info));

			m_current_size++;
		}

		/// <summary>
		/// Remove the latest inserted entry from the stack.
		/// Note: Throws std::out_of_range if the container is empty.
		/// </summary>
		/// <returns></returns>
		const value_t& pop() {
			if (m_current_size <= 0) {
				throw std::out_of_range("pop() called on an empty stack");
			}
			return m_stack[--m_current_size];
		}

		/// <summary>
		/// Returns the latest inserted entry from the stack, but in contrast to pop(), it doesn't alter the internal state.
		/// Note: Throws std::out_of_range if the container is empty.
		/// </summary>
		/// <returns></returns>
		const value_t& top() const {
			if (m_current_size <= 0) {
				throw std::out_of_range("top() called on an empty stack");
			}
			return m_stack[m_current_size - 1];
		}

		/// <summary>
		/// Retrieve the current size of the container
		/// </summary>
		/// <returns></returns>
		size_t size() const noexcept {
			return m_current_size;
		}

		/// <summary>
		/// Clear the container.
		/// </summary>
		/// <returns></returns>
		void clear() noexcept {
			m_current_size = 0;
		}

		// Constructor/destructor.
		move_stack() = default;
		virtual ~move_stack() = default;

		// copy constructor/operator.
		move_stack(const move_stack& _src) {
			std::copy(
				std::begin(_src.m_stack),
				std::end(_src.m_stack),
				std::begin(m_stack)
			);
			m_current_size = _src.m_current_size;
		}
		move_stack& operator=(const move_stack& _src) {
			if (this != &_src) {
				std::copy(
					std::begin(_src.m_stack),
					std::end(_src.m_stack),
					std::begin(m_stack)
				);
				m_current_size = _src.m_current_size;
			}
			return *this;
		}
	};

}

#endif
