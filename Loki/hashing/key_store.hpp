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
#include "position/square.hpp"
#include "position/castle_rights.hpp"
#include "util/arrayops.hpp"
#include "util/rng.hpp"
#include "defs.hpp"
#include <cstdint>

namespace loki::hashing
{
	using hashkey_t = std::uint64_t;

	class hashing_store final
	{
	private:
		constexpr hashing_store(
			util::nested_array_t<hashkey_t, NUM_SIDES, NUM_PIECES, position::NUM_SQUARES> piece_keys,
			std::array<hashkey_t, position::castle_rights::max_value + 1> castling_keys,
			std::array<hashkey_t, position::NUM_FILES> ep_file_keys,
			hashkey_t side_to_move)
			: _piece_keys{ piece_keys }, _castling_keys{ castling_keys }, _ep_file_keys{ ep_file_keys }, _side_to_move_key{ side_to_move }
		{}

	public:
		hashing_store(const hashing_store&) = delete;
		hashing_store& operator=(const hashing_store&) = delete;

		inline constexpr hashkey_t get(piece piece, position::e_square square, side s) const noexcept { return _piece_keys[s][piece][square]; }
		inline constexpr hashkey_t get(const position::castle_rights& rights) const noexcept { return _castling_keys[rights.get()]; }
		inline constexpr hashkey_t get(position::e_square en_passant_square) const noexcept { return _ep_file_keys[position::file_of(en_passant_square)]; }
		inline constexpr hashkey_t side_to_move() const noexcept { return _side_to_move_key; }

		[[nodiscard]]
		static consteval hashing_store initialize()
		{
			auto rng = util::rng::create_consteval();
			hashkey_t side_to_move_key = rng.generate<std::uint64_t>();
			util::nested_array_t<hashkey_t, NUM_SIDES, NUM_PIECES, position::NUM_SQUARES> piece_keys{};
			std::array<hashkey_t, position::castle_rights::max_value + 1> castling_keys{};
			std::array<hashkey_t, position::NUM_FILES> ep_file_keys{};

			for (auto piece = PAWN; piece <= KING; piece++)
			{
				for (auto sq = position::A1; sq <= position::H8; sq++)
				{
					piece_keys[WHITE][piece][sq] = rng.generate<std::uint64_t>();
					piece_keys[BLACK][piece][sq] = rng.generate<std::uint64_t>();
				}
				}
			for (auto i = 0; i <= position::castle_rights::max_value; i++)
				castling_keys[i] = rng.generate<std::uint64_t>();
			for (auto f = position::FILE_A; f <= position::FILE_H; f++)
				ep_file_keys[f] = rng.generate<std::uint64_t>();

			return hashing_store(piece_keys, castling_keys, ep_file_keys, side_to_move_key);
		}
	private:
		util::nested_array_t<hashkey_t, NUM_SIDES, NUM_PIECES, position::NUM_SQUARES> _piece_keys;
		std::array<hashkey_t, position::castle_rights::max_value + 1> _castling_keys;
		hashkey_t _side_to_move_key;

		// en-passant squares can be distinguished by file since only one is possible at a time.
		std::array<hashkey_t, position::NUM_FILES> _ep_file_keys;
	};

	// global object created at compile time.
	inline constexpr hashing_store store = hashing_store::initialize();
}