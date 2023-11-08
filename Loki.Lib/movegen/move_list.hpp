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
#ifndef MOVE_LIST_H
#define MOVE_LIST_H

namespace loki::movegen
{

	template<size_t _Size> requires(_Size > 0)
		class move_list
	{
		EXCEPTION_CLASS(e_moveList, e_lokiError);
	public:
		struct scored_move
		{
			move_t move;
			eValue score;
		};

		using const_iterator = std::array<scored_move, _Size>::const_iterator;
	private:
		std::array<scored_move, _Size> m_movelist;
		size_t m_size;
	public:
		move_list() : m_movelist{ MOVE_NULL }
		{
			m_size = 0;
		}

		inline void add(move_t move, eValue score)
		{
			if (m_size >= _Size)
			{
				throw e_moveList(FORMAT_EXCEPTION_MESSAGE("move_list capacity exceeded"));
			}
			m_movelist[m_size].move = move;
			m_movelist[m_size].score = score;
			m_size++;
		}

		inline void add(size_t from_sq, size_t to_sq, size_t special, size_t promotion_piece, eValue score)
		{
			if (m_size >= _Size)
			{
				throw e_moveList(FORMAT_EXCEPTION_MESSAGE("move_list capacity exceeded"));
			}
			m_movelist[m_size].move = create_move(from_sq, to_sq, special, promotion_piece);
			m_movelist[m_size].score = score;
			m_size++;
		}

		inline void clear() noexcept
		{
			m_size = 0;
		}

		inline size_t size() const noexcept
		{
			return m_size;
		}
		inline const scored_move& operator[](size_t idx) const
		{
			if (idx >= m_size)
			{
				throw e_moveList(FORMAT_EXCEPTION_MESSAGE("Index requested was bigger than the list."));
			}
			return m_movelist[idx];
		}
		inline scored_move& at(size_t inx)
		{
			if (inx >= m_size)
				return m_movelist[m_movelist.size() - 1];
			return m_movelist[inx];
		}
		inline void swap(size_t inx1, size_t inx2)
		{
			if (inx1 >= m_size || inx2 >= m_size)
				throw e_moveList(FORMAT_EXCEPTION_MESSAGE("One of the requested indeces was bigger than the list. inx1 = '{}', inx2 = '{}', m_size = '{}'", inx1, inx2, m_size));
			auto tmp_move = m_movelist[inx1].move;
			auto tmp_score = m_movelist[inx1].score;
			
			m_movelist[inx1].move = m_movelist[inx2].move;
			m_movelist[inx1].score = m_movelist[inx2].score;
			m_movelist[inx2].move = tmp_move;
			m_movelist[inx2].score = tmp_score;
		}

		inline move_t find(std::string move) const
		{
			if (move.length() < 4 || move.length() > 5)
				throw e_moveList(FORMAT_EXCEPTION_MESSAGE("Invalid move notation passed"));
			auto from = from_algebraic(move.substr(0, 2));
			auto to = from_algebraic(move.substr(2, 2));
			auto prom = PIECE_NB;
			if (move.length() > 4)
			{
				switch (std::tolower(move[4]))
				{
				case 'n': prom = KNIGHT; break;
				case 'b': prom = BISHOP; break;
				case 'r': prom = ROOK; break;
				case 'q': prom = QUEEN; break;
				default:
					throw e_moveList(FORMAT_EXCEPTION_MESSAGE("Invalid promotion piece"));
				}
			}

			for (auto i = 0; i < m_size; i++)
			{
				auto internalMove = m_movelist[i].move;

				if (from_sq(internalMove) == from && to_sq(internalMove) == to)
				{
					if (special(internalMove) == PROMOTION && promotion_piece(internalMove) == prom)
						return internalMove;
					else if (special(internalMove) != PROMOTION)
						return internalMove;
				}
			}
			return MOVE_NULL;
		}

		inline move_list(const move_list& _src) : m_size(_src.m_size)
		{
			std::copy(_src.m_movelist.begin(), _src.m_movelist.end(), m_movelist.begin());
		}
		inline move_list& operator=(const move_list& _src)
		{
			if (this != &_src)
			{
				m_size = _src.m_size;
				std::copy(_src.m_movelist.begin(), _src.m_movelist.end(), m_movelist.begin());
			}
			return *this;
		}

		/// <summary>
		/// Check if a move exists in the move-list.
		/// Note: This method potentially loops over all moves in the list, so use it sparingly.
		/// </summary>
		inline bool contains(move_t move) const
		{
			for (auto i = 0; i < m_size; i++)
			{
				if (m_movelist[i].move == move)
					return true;
			}
			return false;
		}

		/*
		Implement begin and end to allow for range-based for loops.
		*/
		const_iterator begin() const noexcept
		{
			return m_movelist.begin();
		}
		const_iterator end() const noexcept
		{
			return m_movelist.begin() + m_size;
		}
	};

}


#endif