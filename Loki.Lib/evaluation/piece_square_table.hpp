#pragma once

namespace loki::evaluation
{
	class PieceSquareTables
	{
	private:
		template<ePiece _P>
		class _PieceSquareTable
		{
			EXCEPTION_CLASS(e_psqtError, e_lokiError);
		private:
			std::array<eValue, SQ_NB> m_table;
		public:
			_PieceSquareTable() : m_table{ VALUE_ZERO }
			{}
			_PieceSquareTable(const std::initializer_list<eValue>& l) : m_table{ VALUE_ZERO }
			{
				if (l.size() != SQ_NB)
					throw e_psqtError(std::format("Error initializing piece-square table for piece type '{}'! Expected a std::initializer_list with size = {} but got {} elements.", (long)_P, (long)SQ_NB, l.size()));
				std::copy(l.begin(), l.end(), m_table.begin());
			}

			template<eSide _S>
			inline eValue get(eSquare sq) const noexcept
			{
				assert(sq >= A1 && sq <= H8);
				return m_table[make_side_relative<_S>(sq)];
			}
		};

		// We keep our tables in a tuple since its values nicely lines up with our numeric piece-values.
		// This means that std::get<PAWN>(m_tables) will actually fetch the pawn table.
		std::tuple<
			_PieceSquareTable<PAWN>,
			_PieceSquareTable<KNIGHT>,
			_PieceSquareTable<BISHOP>,
			_PieceSquareTable<ROOK>,
			_PieceSquareTable<QUEEN>,
			_PieceSquareTable<KING>> m_tables;
	public:
		PieceSquareTables() = default;
		PieceSquareTables(
			const std::initializer_list<eValue>& pawn_list,
			const std::initializer_list<eValue>& knight_list,
			const std::initializer_list<eValue>& bishop_list,
			const std::initializer_list<eValue>& rook_list,
			const std::initializer_list<eValue>& queen_list,
			const std::initializer_list<eValue>& king_list)
		{
			m_tables = std::make_tuple(
				pawn_list,
				knight_list,
				bishop_list,
				rook_list,
				queen_list,
				king_list);
		}

		template<eSide _S, ePiece _P>
		inline eValue get(eSquare sq) const noexcept
		{
			return std::get<static_cast<size_t>(_P)>(m_tables).get<_S>(sq);
		}
	};
}