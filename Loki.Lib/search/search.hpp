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
#pragma once

namespace loki::search
{

	class searcher
	{
	protected:
		EXCEPTION_CLASS(e_searcherError, e_lokiError);
		
		// Ctor params
	protected:
		const eThreadId								m_threadId;
	private:
		const movegen::magics::slider_generator_t	m_sliderGenerator;
		const evaluation::evaluation_params_t		m_evalParams;
		std::shared_ptr<std::atomic_bool>			m_stop; /* Controlled by the main-thread */
		ttPtr_t										m_hashTable;

		// The thread's own params
		position::position_t						m_pos;
		std::unique_ptr<evaluation::evaluator>		m_eval;
		std::shared_ptr<const search_limits>		m_limits;
		std::shared_ptr<search_stats>				m_stats;
	protected:
		searcher() = delete;
		searcher(
			eThreadId									threadId,
			const movegen::magics::slider_generator_t&	sliderGen,
			const evaluation::evaluation_params_t&		params,
			ttPtr_t&									hashTable,
			std::shared_ptr<std::atomic_bool>			stop);

		void search(
			const position::game_state& state,
			std::shared_ptr<const search_limits>& limits);
	
	public:
		virtual uint64_t get_nodes() { return m_stats->info.nodes; }

	private:
		void check_stopped_search();	
		void preprocess_search(
			const position::game_state& state,
			std::shared_ptr<const search_limits>& limits);
		eValue root_search(eDepth depth, eValue alpha, eValue beta);
		eValue alpha_beta(eDepth depth, eValue alpha, eValue beta);
		eValue quiescence(eValue alpha, eValue beta);
		const movegen::move_list_t& generate_quiescence_moves();
		eDepth depth_for_thread(eDepth d) const;

	public:
		inline static constexpr double InternalToCpConversionFactor = 100.0 / 512.0;
		inline static eCentipawnValue to_centipawns(eValue valueInternal) noexcept
		{
			return static_cast<eCentipawnValue>(std::round(InternalToCpConversionFactor * static_cast<double>(valueInternal)));
		}
		inline static long long to_mate(eValue valueInternal) noexcept
		{
			auto addition = (std::abs(valueInternal) % 2 != 0) ? 1 : 0;
			auto score = (((long)VALUE_INF - std::abs(valueInternal)) / 2) + addition;

			return valueInternal > VALUE_MATE ? score : -score;
		}

		virtual ~searcher() {}
	};

}