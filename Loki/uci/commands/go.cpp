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

#include "uci/command_registry.hpp"
#include "uci/move_parsing.hpp"
#include "uci/uci_parser.hpp"
#include "util/exception.hpp"
#include "movegen/move_list.hpp"

#include <cassert>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

using namespace loki;
using namespace loki::uci;
using namespace loki::search;

namespace
{
	bool is_go_option(const std::string& token)
	{
		return token == "searchmoves"
			|| token == "ponder"
			|| token == "wtime"
			|| token == "btime"
			|| token == "winc"
			|| token == "binc"
			|| token == "movestogo"
			|| token == "depth"
			|| token == "nodes"
			|| token == "mate"
			|| token == "movetime"
			|| token == "infinite";
	}

	uint64_t parse_uint64(std::string_view token, std::string_view option)
	{
		uint64_t value = 0;
		const auto* begin = token.data();
		const auto* end = begin + token.size();
		const auto [ptr, ec] = std::from_chars(begin, end, value);
		if (ec != std::errc{} || ptr != end)
			throw_msg<uci_parser::uci_error>("invalid value for '{}': '{}'", option, token);
		return value;
	}

	size_t parse_size(std::string_view token, std::string_view option)
	{
		const auto value = parse_uint64(token, option);
		if (value > static_cast<uint64_t>(std::numeric_limits<size_t>::max()))
			throw_msg<uci_parser::uci_error>("value for '{}' is too large: '{}'", option, token);
		return static_cast<size_t>(value);
	}

	struct parsed_options
	{
		bool searchmoves = false;
		bool ponder = false;
		bool wtime = false;
		bool btime = false;
		bool winc = false;
		bool binc = false;
		bool movestogo = false;
		bool depth = false;
		bool nodes = false;
		bool mate = false;
		bool movetime = false;
		bool infinite = false;
	};

	void require_unique(bool& seen, std::string_view option)
	{
		if (seen)
			throw_msg<uci_parser::uci_error>("duplicate go option: '{}'", option);
		seen = true;
	}

}

class go_command final : public uci_command<go_command>
{
public:
	static std::string name() { return "go"; }

	bool can_execute(const context* ctx) override { return ctx->state == UCI_STATE::Ready; }

	void execute(std::vector<std::string> tokens, context* ctx) override
	{
		assert(can_execute(ctx));
		auto limits = parse_limits(tokens, ctx);
		ctx->engine.search(limits, [&](search_result_t result)
			{
				ctx->state = UCI_STATE::Ready;
				if (!result)
				{
					try
					{
						std::rethrow_exception(result.error());
					}
					catch (const std::exception& e)
					{
						ctx->out << "info string search failed: " << e.what() << std::endl;
					}
				}
			});
		ctx->state = UCI_STATE::Searching;
	}

private:
	using token_iterator = std::vector<std::string>::const_iterator;

	static search::limits parse_limits(const std::vector<std::string>& tokens, context* ctx)
	{
		if (tokens.empty())
			throw_msg<uci_parser::uci_error>("go needs at least one parameter.");

		search::limits limits{};
		std::optional<uint64_t> wtime;
		std::optional<uint64_t> btime;
		std::optional<uint64_t> winc;
		std::optional<uint64_t> binc;
		parsed_options seen{};

		auto it = tokens.cbegin();
		while (it != tokens.cend())
			parse_option(it, tokens.cend(), ctx, limits, wtime, btime, winc, binc, seen);

		if (wtime.has_value() || winc.has_value())
			limits.wtime = std::make_tuple(wtime.value_or(0), winc.value_or(0));
		if (btime.has_value() || binc.has_value())
			limits.btime = std::make_tuple(btime.value_or(0), binc.value_or(0));

		return limits;
	}

	static void parse_option(
		token_iterator& it,
		token_iterator end,
		context* ctx,
		search::limits& limits,
		std::optional<uint64_t>& wtime,
		std::optional<uint64_t>& btime,
		std::optional<uint64_t>& winc,
		std::optional<uint64_t>& binc,
		parsed_options& seen)
	{
		const auto option = *it;
		switch (hash(option))
		{
		case "searchmoves"_hash:
			require_unique(seen.searchmoves, option);
			parse_searchmoves(it, end, ctx, limits);
			break;
		case "ponder"_hash:
			require_unique(seen.ponder, option);
			limits.pondering = true;
			++it;
			break;
		case "wtime"_hash:
			require_unique(seen.wtime, option);
			wtime = parse_numeric_option(it, end);
			break;
		case "btime"_hash:
			require_unique(seen.btime, option);
			btime = parse_numeric_option(it, end);
			break;
		case "winc"_hash:
			require_unique(seen.winc, option);
			winc = parse_numeric_option(it, end);
			break;
		case "binc"_hash:
			require_unique(seen.binc, option);
			binc = parse_numeric_option(it, end);
			break;
		case "movestogo"_hash:
			require_unique(seen.movestogo, option);
			limits.movestogo = parse_size_option(it, end);
			break;
		case "depth"_hash:
			require_unique(seen.depth, option);
			limits.depth = parse_size_option(it, end);
			break;
		case "nodes"_hash:
			require_unique(seen.nodes, option);
			limits.nodes = parse_size_option(it, end);
			break;
		case "mate"_hash:
			require_unique(seen.mate, option);
			limits.mate = parse_size_option(it, end);
			break;
		case "movetime"_hash:
		{
			require_unique(seen.movetime, option);
			const auto movetime = parse_numeric_option(it, end);
			limits.movetime = std::make_tuple(movetime, 0ULL);
			break;
		}
		case "infinite"_hash:
			require_unique(seen.infinite, option);
			limits.infinite = true;
			++it;
			break;
		default:
			throw_msg<uci_parser::uci_error>("invalid go option: '{}'", option);
		}
	}

	static void parse_searchmoves(token_iterator& it, token_iterator end, context* ctx, search::limits& limits)
	{
		++it;
		if (it == end || is_go_option(*it))
			throw_msg<uci_parser::uci_error>("searchmoves needs at least one move.");

		const auto view = ctx->engine.position()->make_view();
		const auto* state = view.game_state();
		movegen::move_list legal_moves;
		ctx->engine.position()->generate_moves(&legal_moves);

		while (it != end && !is_go_option(*it))
		{
			parse_move_token(*it, *state);
			auto move = legal_moves.find(*it);
			if (!move.has_value())
				throw_msg<uci_parser::uci_error>("invalid searchmove: '{}'", *it);

			limits.searchmoves.push_back(*move);
			++it;
		}
	}

	static uint64_t parse_numeric_option(token_iterator& it, token_iterator end)
	{
		const auto option = *it;
		++it;
		if (it == end || is_go_option(*it))
			throw_msg<uci_parser::uci_error>("missing value after '{}'", option);

		return parse_uint64(*(it++), option);
	}

	static size_t parse_size_option(token_iterator& it, token_iterator end)
	{
		const auto option = *it;
		++it;
		if (it == end || is_go_option(*it))
			throw_msg<uci_parser::uci_error>("missing value after '{}'", option);

		return parse_size(*(it++), option);
	}
};

static command_registration<go_command> reg;
