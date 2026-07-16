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
#include "util/exception.hpp"
#include "movegen/move.hpp"
#include "square.hpp"
#include "defs.hpp"

namespace loki::position
{
    /// <summary>
    /// Fast stack used for saving irreversible data when making and unmaking moves.
    /// </summary>
    template<size_t S> requires (S > 0)
    class position_history
    {
        CHILD_EXCEPTION(stack_error, loki_exception);
    public:
        struct move_data
        {
            piece captured = NO_PIECE, moved = NO_PIECE;
            uint8_t castle_rights = 0;
            uint16_t fifty_move = 0; // reduced from size_t to uint16_t to lower copy/memory traffic
            square ep_sq = NO_SQ;
        };
        using entry_t = std::pair<movegen::move_t, move_data>;

        static constexpr size_t MAX_SIZE = S;
    private:
        std::array<entry_t, MAX_SIZE> m_stack;
        size_t m_size;

    public:
        position_history()
            : m_size{0}, m_stack{}
        { }

        inline void push(
            movegen::move_t m,
            piece captured,
            piece moved,
            uint8_t castle_rights,
            size_t fifty_move,
            square ep) noexcept
        {
            if (m_size >= MAX_SIZE)
                throw_msg<stack_error>("push() was called on a full stack!");
            // Use aggregate assignment to minimize individual field writes and allow better optimisation.
            m_stack[m_size++] = { m, { captured, moved, castle_rights, static_cast<uint16_t>(fifty_move), ep } };
        }

        const entry_t& pop()
        {
            if (m_size <= 0)
                throw_msg<stack_error>("pop() was called on an empty stack!");
            return m_stack[--m_size];
        }

        inline size_t size() const noexcept
        {
            return m_size;
        }

        position_history(const position_history&) = default;
        position_history& operator=(const position_history&) = default;
        position_history(position_history&&) = delete;
        position_history& operator=(position_history&&) = delete;
    };
}
