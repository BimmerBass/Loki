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
		PIECE	piece_captured;
		PIECE	piece_moved;
		uint8_t castling_rights;
		size_t	fifty_moves_count;
		SQUARE	en_passant_square;
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
		void insert(move_t move, lost_move_info lost_info) {
			if (m_current_size >= _Size) {
				throw std::out_of_range("stack size limit exceeded");
			}
			m_stack[m_current_size++] = std::make_pair(move, lost_info);
		}
		value_t pop() {
			if (m_current_size <= 0) {
				throw std::out_of_range("pop() called on an empty stack");
			}
			return m_stack[--m_current_size];
		}
		size_t size() const noexcept {
			return m_current_size;
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
