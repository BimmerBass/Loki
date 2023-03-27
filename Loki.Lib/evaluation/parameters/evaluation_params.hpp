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


namespace loki::evaluation
{
	/// <summary>
	/// The evaluation_params interface holds all values that the evaluation function uses.
	/// It is made abstract so we can load values from a file easily in the future which will improve tuning because we wont rely on values in memory and can hot-reload them.
	/// </summary>
	class evaluation_params
	{
		template<typename _Ty, typename... _TArgs> requires std::is_base_of_v<evaluation_params, _Ty>
		friend evaluation_params_t make_params(_TArgs&&... args);
	public:
		// No-one other than make_params can instantiate a restricted_t object, which 
		evaluation_params(restricted_t/*unused*/) {}

		// Material values
		eValue pawn = VALUE_ZERO;
		eValue knight = VALUE_ZERO;
		eValue bishop = VALUE_ZERO;
		eValue rook = VALUE_ZERO;
		eValue queen = VALUE_ZERO;

		// Piece-square tables.
		PieceSquareTables piece_square_tables;

		// Value for the side to move.
		eValue tempo = VALUE_ZERO;


	protected:
		virtual void initialize() = 0;
	};

	/// <summary>
	/// hardcoded_params will be the object Loki uses while actually playing games and analyzing positions.
	/// </summary>
	class hardcoded_params : public evaluation_params
	{
	public:
		hardcoded_params(restricted_t r) : evaluation_params(r) {}

	private:
		void initialize() override;
	};

	/// <summary>
	/// Construct a parameter object.
	/// make_params is the only method that can instantiate these kinds of objects, so this forces a call to initialize() upon first construction.
	/// Subsequent calls is up to the derived class's implementation of the method's visibility.
	/// </summary>
	template<typename _Ty, typename... _TArgs> requires std::is_base_of_v<evaluation_params, _Ty>
	evaluation_params_t make_params(_TArgs&&... args)
	{
		restricted_t r;
		evaluation_params* ptr = new _Ty(r, std::forward<_TArgs>(args)...);
		ptr->initialize();
		return std::shared_ptr<evaluation_params>(std::move(ptr));
	}
}