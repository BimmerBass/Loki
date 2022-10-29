#pragma once
#include "contexts/search_context.h"

namespace loki::uci
{
	class engine_manager
	{
		EXCEPTION_CLASS(e_engineManager, e_lokiError);
	private:
		std::atomic_bool m_quit;
	public:
		// Run the UCI loop and return true upon succesful completion.
		bool run() noexcept;

	private:
		void parse_cmd(const std::string& cmd);
	};

}