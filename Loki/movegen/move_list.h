#ifndef MOVE_LIST_H
#define MOVE_LIST_H

namespace loki::movegen {

	template<size_t _Size> requires(_Size > 0)
	class move_list {
	public:
		struct scored_move {
			move_t move;
			int score;
		};

		using const_iterator = std::array<scored_move, _Size>::const_iterator;
	private:
		std::array<scored_move, _Size> m_movelist;
		size_t m_size;
	public:
		move_list() {
			m_size = 0;
		}

		inline void add(move_t move, int score) {
			if (m_size >= _Size) {
				throw std::runtime_error("move_list capacity exceeded");
			}
			m_movelist[m_size++] = scored_move{
				move,
				score
			};
		}

		inline void clear() noexcept {
			m_size = 0;
		}

		inline size_t size() const noexcept {
			return m_size;
		}

		inline const_iterator begin() const noexcept {
			return m_movelist.begin();
		}
		inline const_iterator end() const noexcept {
			return m_movelist.begin() + m_size;
		}
	};

}


#endif