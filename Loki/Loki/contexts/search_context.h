#pragma once
#include "position_context.h"


namespace loki::uci
{

	class search_context : public context
	{
		position_context position;
		search::search_limits limits;
		std::atomic_bool* quit;
	};

}