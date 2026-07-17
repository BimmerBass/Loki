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

search_thread::search_thread(size_t id) :
	thread_id{id},
	_cv{},
	_mtx{},
	_search_worker{ std::make_unique<search_worker>() },
	_context{std::nullopt}
{
	start_thread();
}

search_thread::search_thread(size_t id, std::unique_ptr<i_search_worker> worker) :
	thread_id{id},
	_cv{},
	_mtx{},
	_search_worker{std::move(worker)},
	_context{std::nullopt}
{
	start_thread();
}

void search_thread::start_thread()
{
	_thread = std::jthread([this](std::stop_token tk)
		{
			this->thread_loop(tk);
		});
}

void search_thread::thread_loop(std::stop_token token)
{
	std::stop_callback callback(token, [&]()
		{
			_cv.notify_all();
		});
	while (true)
	{
		std::unique_lock<std::mutex> lock(_mtx);
		_cv.wait(lock, [&]() { return _context.has_value() || token.stop_requested();});

		// stop requested on the thread's token -> quit
		if (token.stop_requested())
			break;

		// stop not requested, launch a search
		search_context& ctx = _context.value();
		std::unique_ptr<position::search_position> search_pos = ctx.position->clone();
		limits limits = ctx.limits;
		std::stop_token search_stop = (*ctx.limits.stop_source).get_token();
		callback_t callback = std::move(ctx.finished_callback);
		info_sink_t info_sink = std::move(ctx.info_sink);

		// unlock while searching to allow for cancellation.
		lock.unlock();
		auto result = _search_worker->search(
			std::move(search_pos),
			limits,
			search_stop,
			std::move(info_sink));

		// Mark the search idle before publishing completion through the callback.
		// This allows the callback to make the engine Ready without exposing an
		// active search context to the next command.
		lock.lock();
		_context = std::nullopt;
		_callback_running = static_cast<bool>(callback);
		lock.unlock();

		if (callback)
			callback(std::move(result));

		lock.lock();
		_callback_running = false;
		lock.unlock();
		_cv.notify_all();
	}
}
