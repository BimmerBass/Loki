// Loki, a UCI-compliant chess playing software
// Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
// Loki is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "pch.hpp"
#include "Loki/movegen/magics/hardcoded_index.hpp"
#include "Loki/ordering/move_picker.hpp"

namespace ordering_tests
{
	using namespace loki;
	using namespace loki::movegen;
	using namespace loki::movegen::magics;
	using namespace loki::ordering;
	using namespace loki::position;
	using namespace loki::search;

	namespace
	{
		search_position_t make_position(const std::string& fen)
		{
			auto bishop_index = std::make_shared<hardcoded_index<BISHOP>>();
			auto rook_index = std::make_shared<hardcoded_index<ROOK>>();
			return make(game_state::from_fen(fen), bishop_index, rook_index);
		}

		move require_move(search_position& position, const std::string& move_string)
		{
			move_list moves;
			position.generate_moves<ALL>(&moves);
			const auto generated_move = moves.find(move_string);
			REQUIRE(generated_move.has_value());
			return *generated_move;
		}

		template<move_type MT>
		std::vector<move> drain(move_picker<MT>& picker)
		{
			std::vector<move> result;
			while (const auto next = picker.get_next_move())
				result.push_back(*next);
			return result;
		}

		std::vector<uint16_t> raw_moves(const std::vector<move>& moves)
		{
			std::vector<uint16_t> result;
			result.reserve(moves.size());
			for (const auto& generated_move : moves)
				result.push_back(static_cast<uint16_t>(generated_move.get_move()));
			std::ranges::sort(result);
			return result;
		}

		template<move_type MT>
		std::vector<uint16_t> directly_generated_moves(search_position& position)
		{
			move_list moves;
			position.generate_moves<MT>(&moves);

			std::vector<move> generated;
			generated.reserve(moves.size());
			for (const auto& generated_move : moves)
				generated.push_back(generated_move);
			return raw_moves(generated);
		}

		template<move_type MT>
		void require_picker_matches_generation(search_position& position, search_statistics& statistics)
		{
			move_picker<MT> picker{ position, statistics };
			picker.generate_moves();
			const auto picked = drain(picker);

			REQUIRE(raw_moves(picked) == directly_generated_moves<MT>(position));
			for (const auto& generated_move : picked)
			{
				if constexpr (MT == ACTIVE)
					REQUIRE(generated_move.is_active());
				else if constexpr (MT == QUIET)
					REQUIRE_FALSE(generated_move.is_active());
			}
		}
	}

	TEST_CASE("move_picker returns every generated move once and enforces its lifecycle", "[ordering][move_picker]")
	{
		auto position = make_position(constants::START_FEN);
		search_statistics statistics;
		statistics.clear();
		move_picker<ALL> picker{ *position, statistics };

		picker.generate_moves();
		REQUIRE_THROWS_AS(picker.generate_moves(), loki_exception);

		const auto picked = drain(picker);
		REQUIRE(picked.size() == 20);
		REQUIRE(raw_moves(picked) == directly_generated_moves<ALL>(*position));
		REQUIRE_FALSE(picker.get_next_move().has_value());
		REQUIRE_FALSE(picker.get_next_move().has_value());
	}

	TEST_CASE("move_picker template selects the requested move type", "[ordering][move_picker]")
	{
		auto position = make_position("7k/8/8/5p2/4K3/8/8/8 w - - 0 1");
		search_statistics statistics;
		statistics.clear();

		require_picker_matches_generation<ACTIVE>(*position, statistics);
		require_picker_matches_generation<QUIET>(*position, statistics);
		require_picker_matches_generation<ALL>(*position, statistics);
	}

	TEST_CASE("move_picker prioritizes the primary and secondary quiet killers", "[ordering][move_picker][killers]")
	{
		auto position = make_position(constants::START_FEN);
		const auto primary = require_move(*position, "e2e4");
		const auto secondary = require_move(*position, "d2d4");
		search_statistics statistics;
		statistics.clear();
		statistics.killer_moves[position->ply()] = { primary.get_move(), secondary.get_move() };

		move_picker<QUIET> picker{ *position, statistics };
		picker.generate_moves();
		const auto picked = drain(picker);

		REQUIRE(picked.size() == 20);
		REQUIRE(picked[0].get_move() == primary.get_move());
		REQUIRE(picked[1].get_move() == secondary.get_move());
		REQUIRE(picked[0].score() > picked[1].score());
		for (size_t i = 2; i < picked.size(); ++i)
			REQUIRE(picked[1].score() > picked[i].score());
		REQUIRE(raw_moves(picked) == directly_generated_moves<QUIET>(*position));
	}

	TEST_CASE("move_picker returns moves in non-increasing score order", "[ordering][move_picker][scores]")
	{
		auto position = make_position(constants::START_FEN);
		const auto first_killer = require_move(*position, "e2e4");
		const auto second_killer = require_move(*position, "d2d4");
		search_statistics statistics;
		statistics.clear();
		statistics.killer_moves[position->ply()] = { first_killer.get_move(), second_killer.get_move() };

		move_picker<ALL> picker{ *position, statistics };
		picker.generate_moves();
		const auto picked = drain(picker);

		REQUIRE(picked.size() > 1);
		for (size_t i = 1; i < picked.size(); ++i)
			REQUIRE(picked[i - 1].score() >= picked[i].score());
	}

	TEST_CASE("move_picker applies killer bonuses only to quiet moves", "[ordering][move_picker][killers]")
	{
		auto position = make_position("7k/8/8/5p2/4K3/8/8/8 w - - 0 1");
		const auto active_killer = require_move(*position, "e4f5");
		const auto quiet_killer = require_move(*position, "e4d4");
		REQUIRE(active_killer.is_active());
		REQUIRE_FALSE(quiet_killer.is_active());

		search_statistics statistics;
		statistics.clear();
		statistics.killer_moves[position->ply()] = { quiet_killer.get_move(), active_killer.get_move() };

		move_picker<ALL> picker{ *position, statistics };
		picker.generate_moves();
		const auto picked = drain(picker);

		REQUIRE_FALSE(picked.empty());
		const auto first_quiet = std::ranges::find_if(picked, [](const move& generated_move)
			{
				return !generated_move.is_active();
			});
		REQUIRE(first_quiet != picked.end());
		REQUIRE(first_quiet->get_move() == quiet_killer.get_move());
		REQUIRE(first_quiet->score() == KILLER_ONE);
		const auto returned_active = std::ranges::find_if(picked, [&](const move& generated_move)
			{
				return generated_move.get_move() == active_killer.get_move();
			});
		REQUIRE(returned_active != picked.end());
		REQUIRE(returned_active->score() > KILLER_ONE);
		REQUIRE(raw_moves(picked) == directly_generated_moves<ALL>(*position));
	}
}
