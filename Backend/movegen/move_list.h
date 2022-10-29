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
			int score;
		};

		using const_iterator = std::array<scored_move, _Size>::const_iterator;
	private:
		std::array<scored_move, _Size> m_movelist;
		size_t m_size;
	public:
		move_list() : m_movelist{ 0 }
		{
			m_size = 0;
		}

		inline void add(move_t move, int score)
		{
			if (m_size >= _Size)
			{
				throw e_moveList(FORMAT_EXCEPTION_MESSAGE("move_list capacity exceeded"));
			}
			m_movelist[m_size].move = move;
			m_movelist[m_size].score = score;
			m_size++;
		}

		inline void add(size_t from_sq, size_t to_sq, size_t special, size_t promotion_piece, int score)
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
		const scored_move& operator[](size_t idx) const
		{
			if (idx >= m_size)
			{
				throw e_moveList(FORMAT_EXCEPTION_MESSAGE("Index requested was bigger than the list."));
			}
			return m_movelist[idx];
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
	};

}


#endif