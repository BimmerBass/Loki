// Loki, a UCI-compliant chess playing software
// Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
// Loki is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Loki is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "search_thread.hpp"

using namespace loki;
using namespace loki::search;
using namespace loki::position;

void loki::search::thread_loop(std::stop_token token, worker_thread& worker)
{
	// register a callback such that listeners on the condition variable
	// will be notified when the stop source is triggered.
	std::stop_callback callback(token, [&worker]() { worker._cv.notify_all(); });

	for (;;)
	{
		try
		{
			search_command task = worker.wait_for_work(token);
			std::stop_token search_stop = (*task.limits.stop_source).get_token();

			{
				worker_thread::guard_t lock(worker._mtx);
				worker.current_limits = &task.limits;
			}

			auto result = worker._search_worker->search(
				std::move(task.position),
				task.limits,
				search_stop,
				std::move(task.sink));

			{
				worker_thread::guard_t lock(worker._mtx);
				worker._task = std::nullopt;
				worker.current_limits = nullptr;
				task.promise.set_value(std::move(result));
			}

			worker._cv.notify_all();
		}
		catch (worker_thread::stop_requested_exception)
		{
			break;
		}
	}
}

worker_thread::worker_thread(size_t id, std::unique_ptr<i_search_worker> worker) :
	_id{ id },
	_mtx{},
	_cv{},
	_search_worker{ std::move(worker) },
	_task{ std::nullopt }
{
	_thread = std::jthread(thread_loop, std::ref(*this));
}

worker_thread::worker_thread(size_t id) : worker_thread(id, std::make_unique<search_worker>())
{}


std::future<search_result_t> worker_thread::search(
	const position::search_position_t& position,
	limits limits,
	info_sink_t sink)
{
	if (!position)
		throw thread_exception("search requested with null position");

	std::future<search_result_t> future;
	{
		guard_t lock(_mtx);
		if (_task)
			throw thread_exception("search requested on working thread");
		_task.emplace(search_command(position, limits, std::move(sink)));
		(*_task).limits.stop_source = std::stop_source{};

		future = (*_task).promise.get_future();
	}
	_cv.notify_one();
	return future;
}

void worker_thread::stop_search() noexcept
{
	guard_t lock(_mtx);
	if (!_task)
		return;
	auto& ss = (*_task).limits.stop_source;
	(*ss).request_stop();
}

void worker_thread::wait_for_finished_search()
{
	lock_t lock(_mtx);
	if (!_task)
		return;
	_cv.wait(lock, [&]() { return !_task; });
}

void worker_thread::clear_context()
{
	guard_t lock(_mtx);
	if (_task)
		throw thread_exception("cannot clear state on running thread");
	_search_worker->newgame_clear();
}
void worker_thread::ponderhit()
{
	guard_t lock(_mtx);
	if (!_task || current_limits == nullptr)
		throw thread_exception("ponderhit requested on idle thread");
	current_limits->start_time = limits::clock_t::now();
	current_limits->pondering.store(false, std::memory_order_release);
}

search_command&& worker_thread::wait_for_work(std::stop_token tk)
{
	lock_t lock(_mtx);
	_cv.wait(lock, [&]() { return _task || tk.stop_requested(); });

	if (tk.stop_requested())
		throw stop_requested_exception();

	return std::move(*_task);
}


void worker_thread::join() noexcept
{
	stop_search();

	lock_t lock(_mtx);
	if (_thread.joinable())
	{
		_thread.request_stop();

		lock.unlock();
		_thread.join();
	}
}
