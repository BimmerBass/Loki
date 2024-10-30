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
#include "loki_context.hpp"
#include "versioninfo.hpp"

namespace loki::uci
{
	loki_context::loki_context(std::ostream& os) :
		m_os{ os }
	{}

	void loki_context::uci() const
	{
		m_os << std::format("id name {} {}", NAME, VERSION) << std::endl;
		m_os << std::format("id author {}", AUTHOR) << std::endl;
		// options
		m_os << "uciok" << std::endl;
	}

	void loki_context::debug() const
	{
		throw_msg<not_implemented_error>("not implemented");
	}

	void loki_context::isready() const
	{
		// TODO: Also output this while searching if we're asked to. Perhaps have the main_thread listening for inputs...
		m_os << "readyok" << std::endl;
	}
	void loki_context::setoption(std::string name, std::optional<std::string> value)
	{
		throw_msg<not_implemented_error>("not implemented");
	}
	void loki_context::register_() const
	{
		throw_msg<not_implemented_error>("not implemented");
	}
	void loki_context::ucinewgame()
	{
		throw_msg<not_implemented_error>("not implemented");
	}
	void loki_context::position(const position::game_state* /**/)
	{
		throw_msg<not_implemented_error>("not implemented");
	}
	void loki_context::go(const search::limits* /**/)
	{
		throw_msg<not_implemented_error>("not implemented");
	}
	void loki_context::stop()
	{
		throw_msg<not_implemented_error>("not implemented");
	}
	void loki_context::ponderhit()
	{
		throw_msg<not_implemented_error>("not implemented");
	}
}