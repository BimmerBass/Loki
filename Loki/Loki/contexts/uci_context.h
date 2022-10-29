#pragma once
#include "Backend/loki.pch.h"
#include <map>

namespace loki::uci
{
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

	inline const std::map<std::string, command_type> command_map = {
		{"uci", UCI},
		{"debug", DEBUG},
		{"isready", ISREADY},
		{"setoption", SETOPTION},
		{"register", REGISTER},
		{"ucinewgame", UCINEWGAME},
		{"position", POSITION},
		{"go", GO},
		{"stop", STOP},
		{"ponderhit", PONDERHIT},
		{"quit", QUIT}
	};

	class context
	{
	protected:
		EXCEPTION_CLASS(e_uciError, e_lokiError);

		command_type type;
		std::string command_parameters;

	public:
		context(command_type _type, std::string _parms) : type(_type), command_parameters(_parms) {}
	};

}