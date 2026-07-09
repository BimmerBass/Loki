// Loki, a UCI-compliant chess playing software
// Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
//
// Loki is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Loki is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#pragma once
#include <atomic>
#include <iostream>
#include <chrono>
#include <thread>
#include <expected>

#include "defs.hpp"
#include "position/search_position.hpp"
#include "limits.hpp"
#include "util/exception.hpp"

namespace loki::search
{
	using search_result_t = std::expected<score_t, std::exception_ptr>;

	/// <summary>
	/// search_worker is the class responsible for the actual alpha-beta search.
	/// </summary>
	class search_worker final
	{
	public:
		// TODO: search_worker() should take global references that will not be deleted.

		search_result_t search(
			std::unique_ptr<position::search_position> position,
			const limits& limits,
			std::stop_token stop_token) noexcept
		{
			try
			{
				throw_msg<not_implemented_error>("search not implemented");
			}
			catch (...)
			{
				return std::unexpected(std::current_exception());
			}
		}
	};
}