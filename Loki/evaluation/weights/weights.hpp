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
#include "evaluation/terms.hpp"

namespace loki::evaluation
{
	using weight_t = int32_t;
	using feature_set_t = std::array<weight_t, feature_count()>;

	class weights final
	{
		friend class i_weight_source;
	public:
		constexpr weight_t operator[](id_t id) const noexcept
		{
			return weights_[id];
		}

	private:
		feature_set_t weights_{};
	};

	class i_weight_source
	{
	public:
		~i_weight_source() = default;

		[[nodiscard]]
		virtual weights load() const = 0;

	protected:
		static constexpr void set_weight(weights& w, id_t id, weight_t value)
		{
			w.weights_[id] = value;
		}
	};
}