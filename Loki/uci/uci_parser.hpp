//
//	Loki, a UCI-compliant chess playing software
//	Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
//	Loki is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	Loki is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
#pragma once
#include <functional>
#include <map>
#include <memory>

#include "context_interface.hpp"

namespace loki::uci
{
	class uci_parser
	{
	public:
		CHILD_EXCEPTION(uci_error, loki_exception);
		CHILD_EXCEPTION(quit_exception, loki_exception);
	private:
		using tokens_t = std::vector<std::string>;
		
		std::shared_ptr<context_interface> m_context;
		std::map<std::string, std::function<void(const tokens_t&)>> m_uciCallbacks;
	public:
		/// <summary>
		/// Initialize 
		/// </summary>
		/// <param name="ptr"></param>
		uci_parser(const std::shared_ptr<context_interface>& ptr);

		/// <summary>
		/// Main loop which implements the UCI protocol.
		/// </summary>
		/// <returns>zero if no error occurs. non-zero otherwise.</returns>
		int uci_loop() noexcept;

	public:
		void parse_uci(const tokens_t&) const { m_context->uci(); }
		void parse_isready(const tokens_t&) const { m_context->isready(); }
		void parse_setoption(const tokens_t&);
		void parse_ucinewgame(const tokens_t&) { m_context->ucinewgame(); }
		void parse_position(const tokens_t&);
		void parse_go(const tokens_t&);
		void parse_quit(const tokens_t&) { throw quit_exception(); }
		void parse_stop(const tokens_t&) { m_context->stop(); }

		// unused/misc
		void parse_debug(const tokens_t&) {}
		void parse_register(const tokens_t&) {}
		void parse_ponderhit(const tokens_t&) { m_context->ponderhit(); }

		void parse_printpos(const tokens_t&);
		void parse_perft(const tokens_t&);
	};
}