#pragma once
#include "Backend/loki.pch.h"
#include <syncstream>

namespace loki::uci
{

	class engine_manager
	{
		EXCEPTION_CLASS(e_engineManager, e_lokiError);

		enum command_type
		{
			UCI,
			DEBUG,
			ISREADY,
			SETOPTION,
			REGISTER,
			UCINEWGAME,
			POSITION,
			GO,
			STOP,
			PONDERHIT,
			QUIT,
			UCI_NONE
		};
	private:
		std::atomic_bool m_quit;
	public:
		// Run the UCI loop and return true upon succesful completion.
		bool run() noexcept;

	private:

		command_type get_type(std::string cmd);
		void parse_cmd(const std::string& cmd);
		
	};

}