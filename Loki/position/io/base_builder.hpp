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
#include <memory>

namespace loki::position::io
{
	template<typename TypeFrom, typename TypeTo>
	class base_builder
	{
	protected:
		std::shared_ptr<TypeFrom> m_resource;
		std::shared_ptr<TypeTo> m_product;

		virtual void reset_internal() {}
	public:
		base_builder()
		{
			reset(nullptr, nullptr);
		}
		virtual ~base_builder() {}

		// interface
		virtual base_builder& piece_placements() = 0;
		virtual base_builder& side_to_move() = 0;
		virtual base_builder& castling_ability() = 0;
		virtual base_builder& en_passant_square() = 0;
		virtual base_builder& halfmove_clock() = 0;
		virtual base_builder& fullmove_clock() = 0;
		// interface end

		inline base_builder& reset(
			std::shared_ptr<TypeFrom> resource,
			std::shared_ptr<TypeTo> product)
		{
			m_resource = resource;
			m_product = product;
			reset_internal();
			return *this;
		}

		/// <summary>
		/// Returns the built product and resets the object.
		/// </summary>
		/// <returns>A std::shared_ptr to the product</returns>
		std::shared_ptr<TypeTo> get_product()
		{
			std::shared_ptr<TypeTo> retPtr = m_product;
			reset(nullptr, nullptr);
			return retPtr;
		}
	};
}