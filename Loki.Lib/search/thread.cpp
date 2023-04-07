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
#include "loki.pch.hpp"


namespace loki::search
{
	search_thread::search_thread(eThreadId threadId, const movegen::magics::slider_generator_t& sliderGen, const evaluation::evaluation_params_t& params, ttPtr_t& hashTable, std::shared_ptr<std::atomic_bool> stop) :
		searcher(threadId, sliderGen, params, hashTable, stop),
		m_state{},
		m_limits{nullptr},
		m_sleep(true),
		m_stop(false),
		m_thread([this]() { thread_loop(); })
	{	}

	search_thread::~search_thread()
	{
		// Wake up thread in thread_loop.
		request_stop();
		m_thread.join();
	}

	void search_thread::begin_search(
		const position::game_state& state,
		std::shared_ptr<const search_limits> limits)
	{
		std::unique_lock<std::mutex> lock(m_mtx); // Need a proper lock in case the method throws.
		if (!m_sleep)
			throw e_searchThread("Search was requested on active thread");

		m_state = state;
		m_limits = limits;
		m_sleep = false;

		lock.unlock();
		m_cv.notify_one();
	}

	void search_thread::search_internal(
		const position::game_state& state,
		std::shared_ptr<const search_limits> limits)
	{
		search(state, limits);
	}

	void search_thread::thread_loop()
	{
		while (true)
		{
			// Wait until we're requested to wake up again.
			std::unique_lock<std::mutex> lock(m_mtx);
			m_sleep = true;
			m_cv.notify_one();
			m_cv.wait(lock, [&]() { return !m_sleep || m_stop; });

			if (m_stop)
				break;

			// If we weren't requested to stop, we should search.
			lock.unlock();
			search_internal(m_state, m_limits);
		}
	}

	void search_thread::wait_for_finished_search()
	{
		std::unique_lock<std::mutex> lock(m_mtx);
		m_cv.wait(lock, [&]() { return m_sleep; });
	}

	void search_thread::request_stop()
	{
		m_mtx.lock();
		m_stop = true;
		m_mtx.unlock();
		m_cv.notify_one();
	}

	main_thread::main_thread(const movegen::magics::slider_generator_t& sliderGen, const evaluation::evaluation_params_t& params, ttPtr_t& hashTable)
		: _stopFlag(), search_thread(MAIN_THREAD, sliderGen, params, hashTable, m_stopFlag),
		m_sliderGen(sliderGen), m_evalParams(params), m_workerThreads{}, m_quitPtr{nullptr}, m_hashTable{hashTable}
	{
	}

	main_thread::~main_thread()
	{
		m_stopFlag->store(true, std::memory_order_relaxed);
		request_stop();
		m_workerThreads.clear();
	}

	/// <summary>
	/// Override of search_thread::begin_search. Will begin search, start worker threads, and block until search is finished.
	/// TODO: Perhaps try to start the main thread only after all of the workers in the hopes of them filling up the TT, and thus making the main-thread's work more efficient, allowing for greater depth.
	/// </summary>
	void main_thread::begin_search(
		const position::game_state& state,
		std::shared_ptr<const search_limits> limits)
	{
		// Begin searching with the main thread.
		search_thread::begin_search(state, limits);

		for (auto& thread : m_workerThreads)
			thread->begin_search(state, limits);
		wait_for_finished_search();

		// If a quitException was thrown, rethrow to stop program execution.
		if (m_quitPtr)
			std::rethrow_exception(m_quitPtr);
	}

	/// <summary>
	/// When requesting the search thread to stop, all workers should also be stopped.
	/// </summary>
	void main_thread::request_stop()
	{
		// Stop main thread
		search_thread::request_stop();

		// Stop workers.
		for (auto& thread : m_workerThreads)
			thread->request_stop();
	}

	/// <summary>
	/// When waiting for the main thread to stop, we should also wait for the workers to stop.
	/// </summary>
	void main_thread::wait_for_finished_search()
	{
		search_thread::wait_for_finished_search();

		for (auto& thread : m_workerThreads)
			thread->wait_for_finished_search();
	}

	/// <summary>
	/// Set the amount of threads to use.
	/// </summary>
	/// <param name="count">Requested thread count.</param>
	void main_thread::set_thread_count(size_t count)
	{
		if (count < 1)
			throw e_mainThread(std::format("A non-zero positive value was expected, but 'count' was equal to '{}'", count));
		count--; // Main is a search thread itself.
		if (m_workerThreads.size() > count)
			kill_last_n(m_workerThreads.size() - count);
		
		while (m_workerThreads.size() < count)
		{
			eThreadId last_id = m_workerThreads.empty() ? MAIN_THREAD : m_workerThreads.back()->Id();
			auto new_thread = std::make_unique<search_thread>(last_id++, m_sliderGen, m_evalParams, m_hashTable, m_stopFlag);
			m_workerThreads.push_back(std::move(new_thread));
		}
	}

	/// <summary>
	/// Stop the last n threads in our list.
	/// </summary>
	void main_thread::kill_last_n(size_t n)
	{
		if (n > m_workerThreads.size())
			throw e_mainThread(std::format("m_workerThreads contains '{}' elements, but kill_last_n was called with n = '{}'", m_workerThreads.size(), n));
		// Firstly request the threads to stop, and then delete them from the list.
		for (auto i = 1; i <= n; i++)
			m_workerThreads[m_workerThreads.size() - i]->request_stop();
		m_workerThreads.erase(m_workerThreads.end() - n, m_workerThreads.end());
	}

	/// <summary>
	/// Get the maximum amount of separate threads the system can spawn.
	/// If the value can't be fetched, fall back to a default of eight.
	/// </summary>
	int main_thread::max_thread_count()
	{
		auto cnt = std::thread::hardware_concurrency();
		return cnt <= 0 ? 8 : cnt;
	}

	void main_thread::search_internal(
		const position::game_state& state,
		std::shared_ptr<const search_limits> limits)
	{
		try
		{
			search(state, limits);
		}
		catch (uci::engine_manager::e_quitException&) // Signal from search to quit.
		{
			m_quitPtr = std::current_exception();
		}
	}

	uint64_t main_thread::get_nodes()
	{
		auto nodes = search_thread::get_nodes();

		for (auto& thread : m_workerThreads)
			nodes += thread->get_nodes();
		return nodes;
	}
}