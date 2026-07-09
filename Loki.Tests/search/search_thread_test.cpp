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

#include "pch.hpp"
#include "Loki/search/search_thread.hpp"
#include "Loki/movegen/magics/hardcoded_index.hpp"

namespace search_tests
{
	using namespace std::chrono_literals;
	using namespace loki;
	using namespace loki::movegen;
	using namespace loki::movegen::magics;
	using namespace loki::position;
	using namespace loki::search;

	namespace
	{
		search_position_t make_start_position()
		{
			auto state = game_state::from_fen(constants::START_FEN);
			auto bishop_index = std::make_shared<hardcoded_index<BISHOP>>();
			auto rook_index = std::make_shared<hardcoded_index<ROOK>>();

			return make(state, bishop_index, rook_index);
		}
	}

	TEST_CASE("search_thread invokes callback and becomes reusable", "[search][search_thread]")
	{
		search_thread thread{ 0 };
		auto position = make_start_position();

		std::mutex mutex;
		std::condition_variable callback_called;
		size_t callback_count = 0;

		auto callback = [&](search_result_t)
			{
				std::scoped_lock lock{ mutex };
				++callback_count;
				callback_called.notify_all();
			};

		thread.search(position, limits{}, callback);
		{
			std::unique_lock lock{ mutex };
			REQUIRE(callback_called.wait_for(lock, 1s, [&]() { return callback_count == 1; }));
		}
		thread.wait_for_finished_search();

		thread.search(position, limits{}, callback);
		{
			std::unique_lock lock{ mutex };
			REQUIRE(callback_called.wait_for(lock, 1s, [&]() { return callback_count == 2; }));
		}
		thread.wait_for_finished_search();
	}

	TEST_CASE("search_thread rejects overlapping searches", "[search][search_thread]")
	{
		search_thread thread{ 0 };
		auto position = make_start_position();

		std::mutex mutex;
		std::condition_variable callback_entered;
		std::condition_variable callback_can_return;
		bool callback_was_entered = false;
		bool allow_callback_to_return = false;

		thread.search(position, limits{}, [&](search_result_t)
			{
				std::unique_lock lock{ mutex };
				callback_was_entered = true;
				callback_entered.notify_all();
				callback_can_return.wait(lock, [&]() { return allow_callback_to_return; });
			});

		{
			std::unique_lock lock{ mutex };
			REQUIRE(callback_entered.wait_for(lock, 1s, [&]() { return callback_was_entered; }));
		}

		REQUIRE_THROWS_AS(thread.search(position, limits{}, nullptr), search_thread::thread_exception);

		{
			std::scoped_lock lock{ mutex };
			allow_callback_to_return = true;
		}
		callback_can_return.notify_all();
		thread.wait_for_finished_search();
	}

	TEST_CASE("search_thread rejects stopping while idle", "[search][search_thread]")
	{
		search_thread thread{ 0 };

		REQUIRE_THROWS_AS(thread.stop_search(), search_thread::thread_exception);
	}
}
