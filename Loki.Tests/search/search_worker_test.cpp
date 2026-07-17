// Loki, a UCI-compliant chess playing software
// Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)

#include "pch.hpp"
#include "Loki/search/search_worker.hpp"
#include "Loki/movegen/magics/hardcoded_index.hpp"

namespace search_tests
{
	using namespace loki;
	using namespace loki::movegen;
	using namespace loki::movegen::magics;
	using namespace loki::position;
	using namespace loki::search;

	namespace
	{
		struct info_event
		{
			size_t depth;
			size_t selective_depth;
			std::chrono::milliseconds time;
			size_t nodes;
			size_t nps;
			std::vector<move> pv;
		};

		struct sink_state
		{
			std::vector<info_event> events;
			std::vector<std::pair<move, std::optional<move>>> bestmoves;
		};

		class recording_sink final : public info_sink
		{
		public:
			explicit recording_sink(std::shared_ptr<sink_state> state) : _state{ std::move(state) }
			{
			}

			void info(
				size_t depth,
				search_score_t,
				size_t seldepth,
				std::chrono::milliseconds time,
				size_t nodes,
				size_t nps,
				std::vector<move> pv) override
			{
				_state->events.push_back({ depth, seldepth, time, nodes, nps, std::move(pv) });
			}

			void bestmove(move best, std::optional<move> ponder) override
			{
				_state->bestmoves.emplace_back(best, ponder);
			}

		private:
			std::shared_ptr<sink_state> _state;
		};

		search_position_t make_position(const std::string& fen)
		{
			auto bishop_index = std::make_shared<hardcoded_index<BISHOP>>();
			auto rook_index = std::make_shared<hardcoded_index<ROOK>>();
			return make(game_state::from_fen(fen), bishop_index, rook_index);
		}

		search_result_t run_search(
			search_worker& worker,
			const search_position_t& position,
			const limits& search_limits,
			const std::shared_ptr<sink_state>& state,
			std::stop_token token = {})
		{
			return worker.search(
				position->clone(),
				search_limits,
				token,
				std::make_unique<recording_sink>(state));
		}

		void require_report_invariants(
			const search_position_t& position,
			const sink_state& state)
		{
			REQUIRE_FALSE(state.events.empty());
			REQUIRE(state.bestmoves.size() == 1);

			const auto& last = state.events.back();
			REQUIRE(last.nodes > 0);
			REQUIRE(last.time.count() >= 0);
			REQUIRE_FALSE(last.pv.empty());
			REQUIRE(last.pv.size() <= constants::MAX_DEPTH);

			auto legal_position = position->clone();
			move_list legal_moves;
			legal_position->generate_all_legals(&legal_moves);
			REQUIRE(std::ranges::find(legal_moves, last.pv.front()) != legal_moves.end());

			const auto& [best, ponder] = state.bestmoves.front();
			REQUIRE(best == last.pv.front());
			if (last.pv.size() >= 2)
				REQUIRE(ponder == last.pv[1]);
			else
				REQUIRE_FALSE(ponder.has_value());
		}
	}

	TEST_CASE("search_worker converts invalid input into an error result", "[search][search_worker]")
	{
		search_worker worker;
		limits search_limits{};
		search_limits.depth = 1;

		SECTION("null position")
		{
			auto result = worker.search(
				nullptr,
				search_limits,
				std::stop_token{},
				std::make_unique<null_sink>());
			REQUIRE_FALSE(result.has_value());
		}

		SECTION("null sink")
		{
			auto position = make_position(constants::START_FEN);
			auto result = worker.search(
				position->clone(),
				search_limits,
				std::stop_token{},
				nullptr);
			REQUIRE_FALSE(result.has_value());
		}
	}

	TEST_CASE("search_worker reports structural invariants for either side to move", "[search][search_worker]")
	{
		const std::array<std::string, 2> fens{
			constants::START_FEN,
			"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"
		};

		for (const auto& fen : fens)
		{
			CAPTURE(fen);
			search_worker worker;
			auto position = make_position(fen);
			auto state = std::make_shared<sink_state>();
			limits search_limits{};
			search_limits.depth = 1;

			auto result = run_search(worker, position, search_limits, state);

			REQUIRE(result.has_value());
			REQUIRE(state->events.size() == 1);
			REQUIRE(state->events.front().depth == 1);
			require_report_invariants(position, *state);
		}
	}

	TEST_CASE("search_worker reports each completed iterative-deepening depth", "[search][search_worker]")
	{
		search_worker worker;
		auto position = make_position(constants::START_FEN);
		auto state = std::make_shared<sink_state>();
		limits search_limits{};
		search_limits.depth = 2;

		auto result = run_search(worker, position, search_limits, state);

		REQUIRE(result.has_value());
		REQUIRE(state->events.size() == 2);
		REQUIRE(state->events[0].depth == 1);
		REQUIRE(state->events[1].depth == 2);
		REQUIRE(state->events[1].nodes >= state->events[0].nodes);
		REQUIRE(state->events[1].selective_depth >= state->events[0].selective_depth);
		require_report_invariants(position, *state);
	}

	TEST_CASE("search_worker resets position ply before searching", "[search][search_worker]")
	{
		search_worker worker;
		auto position = make_position(constants::START_FEN);
		position->ply() = static_cast<ply_t>(17);
		auto state = std::make_shared<sink_state>();
		limits search_limits{};
		search_limits.depth = 1;

		auto result = run_search(worker, position, search_limits, state);

		REQUIRE(result.has_value());
		REQUIRE(state->events.size() == 1);
		REQUIRE(state->events.front().selective_depth == 1);
	}

	TEST_CASE("search_worker honors the root searchmoves filter", "[search][search_worker]")
	{
		search_worker worker;
		auto position = make_position(constants::START_FEN);
		auto state = std::make_shared<sink_state>();
		move_list legal_moves;
		position->generate_all_legals(&legal_moves);
		REQUIRE(legal_moves.size() >= 2);

		limits search_limits{};
		search_limits.depth = 1;
		search_limits.searchmoves = { legal_moves[0], legal_moves[1] };

		auto result = run_search(worker, position, search_limits, state);

		REQUIRE(result.has_value());
		require_report_invariants(position, *state);
		const auto best = state->bestmoves.front().first;
		REQUIRE(std::ranges::find(search_limits.searchmoves, best) != search_limits.searchmoves.end());
	}

	TEST_CASE("search_worker handles a pre-cancelled search without publishing output", "[search][search_worker]")
	{
		search_worker worker;
		auto position = make_position(constants::START_FEN);
		auto state = std::make_shared<sink_state>();
		limits search_limits{};
		search_limits.depth = 1;
		std::stop_source stop_source;
		stop_source.request_stop();

		auto result = run_search(
			worker,
			position,
			search_limits,
			state,
			stop_source.get_token());

		REQUIRE_FALSE(result.has_value());
		REQUIRE(state->events.empty());
		REQUIRE(state->bestmoves.empty());
	}
}
