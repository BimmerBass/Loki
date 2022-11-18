#pragma once

#define REGISTER_CALLBACK(func) m_callbackMap[std::string(#func).substr(6)] = [this](const std::string& s) { func(s);}

namespace loki::uci
{
	class engine_manager
	{
		EXCEPTION_CLASS(e_engineManager, e_lokiError);
	private:
		inline static const std::string VERSION = "4.0";
		inline static const std::string NAME = "Loki";
		inline static const std::string AUTHOR = "Niels Abildskov";


		std::atomic_bool m_quit;
		std::map<std::string, std::function<void(const std::string&)>> m_callbackMap;
	public:
		engine_manager();


		// Run the UCI loop and return true upon succesful completion.
		bool run() noexcept;

	private:
		void parse_uci(const std::string& uci);
		void parse_isready(const std::string& uci);
		void parse_setoption(const std::string& uci) {};
		void parse_ucinewgame(const std::string& uci) {};
		void parse_position(const std::string& uci) {};
		void parse_go(const std::string& uci) {};
		void parse_stop(const std::string& uci) {};
		void parse_quit(const std::string& uci) {};

		// Parse methods that are intentionally not implemented, but may be in the future.
		void parse_debug(const std::string& uci) {};
		void parse_register(const std::string& uci) {};
		void parse_ponderhit(const std::string& uci) {};
	};

}