#pragma once
#include "position_context.h"


namespace loki::uci
{

	struct search_context
	{
		position_context position;
		search::search_limits limits;
		std::atomic_bool* quit;
	};

}