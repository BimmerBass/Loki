#include "movegen/move.hpp"
#include "position/game_state.hpp"

namespace loki::uci
{
	/// <summary>
	/// Parse a move tokens and infer any attributes it should have given the current state
	/// </summary>
	/// <param name="token"></param>
	/// <param name="state"></param>
	/// <returns></returns>
	movegen::move parse_move_token(const std::string& token, const position::game_state& state);
}