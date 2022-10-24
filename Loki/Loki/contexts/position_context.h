#pragma once
#include "Backend/loki.pch.h"

namespace loki::uci
{
	struct position_context
	{
		std::string fen;
		std::vector<std::string> moves;

		position::game_state_t game_state;
	};
}