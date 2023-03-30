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
	class search_thread : public searcher
	{
	protected:
		EXCEPTION_CLASS(e_searchThread, e_searcherError);
	private:
		std::thread m_thread;
		std::mutex m_mtx;
		std::condition_variable m_cv;

		bool m_sleep, m_stop;
		position::game_state m_state;
		std::shared_ptr<const search_limits> m_limits;
	public:
		search_thread() = delete;
		search_thread(
			eThreadId threadId,
			const movegen::magics::slider_generator_t& sliderGen,
			const evaluation::evaluation_params_t& params,
			std::shared_ptr<std::atomic_bool> stop);
		virtual ~search_thread();

		// Request the thread to begin searching.
		virtual void begin_search(
			const position::game_state& state,
			std::shared_ptr<const search_limits> limits);
		virtual void request_stop();
		virtual void wait_for_finished_search();

		eThreadId Id() const noexcept { return m_threadId; }
	protected:
		virtual void search_internal(
			const position::game_state& state,
			std::shared_ptr<const search_limits> limits);
	private:
		void thread_loop();
	};

	/// <summary>
	/// _stopFlag is a class that holds a pointer to an atomic boolean variable. It is a class of its own so main_thread can 1) Construct its stop-flag bfore its search_thread code, and 2) Hold on to said stop-flag, 
	/// which is needed for creating new search-threads that are coupled to the main-thread.
	/// </summary>
	class _stopFlag
	{
	public:
		virtual ~_stopFlag() {}
	protected:
		_stopFlag()
		{
			m_stopFlag = std::make_shared<std::atomic_bool>(false);
		}
		std::shared_ptr<std::atomic_bool> m_stopFlag;
	};

	/// <summary>
	/// Main thread will execute a search as well as manage all the worker threads.
	/// NOTE: Access to main_thread is *NOT* thread safe
	/// </summary>
	class main_thread : public _stopFlag, public search_thread
	{
		EXCEPTION_CLASS(e_mainThread, e_searchThread);
	private:
		using thread_t = std::unique_ptr<search_thread>;
		std::vector<thread_t> m_workerThreads;
		std::exception_ptr m_quitPtr;


		// Used for constructing new threads
		movegen::magics::slider_generator_t m_sliderGen;
		evaluation::evaluation_params_t m_evalParams;
	public:
		main_thread(const movegen::magics::slider_generator_t& sliderGen,
			const evaluation::evaluation_params_t& params);
		~main_thread();
		
		/// <summary>
		/// Set the amount of threads to use.
		/// </summary>
		/// <param name="count">Requested thread count.</param>
		void set_thread_count(size_t count);

		/// <summary>
		/// Override of search_thread::begin_search. Will begin search, start worker threads, and block until search is finished.
		/// </summary>
		void begin_search(
			const position::game_state& state,
			std::shared_ptr<const search_limits> limits) override;

		/// <summary>
		/// When requesting the search thread to stop, all workers should also be stopped.
		/// </summary>
		void request_stop() override;

		/// <summary>
		/// When waiting for the main thread to stop, we should also wait for the workers to stop.
		/// </summary>
		void wait_for_finished_search() override;

		// Default amount of threads used for searching.
		inline static constexpr int default_thread_count = 1;
		inline static constexpr int min_thread_count = 1;
		static int max_thread_count();
	private:
		void kill_last_n(size_t n);

		void search_internal(
			const position::game_state& state,
			std::shared_ptr<const search_limits> limits) override;
	};
}