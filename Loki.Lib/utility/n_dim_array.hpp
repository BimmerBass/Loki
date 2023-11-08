#pragma once

namespace loki::utility
{
	template<typename _Ty, size_t _Size1, size_t... _SizeArgs>
	class NDimensionalArray
	{
		EXCEPTION_CLASS(e_nDimArray, e_lokiError);
		EXCEPTION_CLASS(e_outOfRange, e_nDimArray)
	public:
		using type = std::remove_cvref_t<_Ty>;
		using child_type = NDimensionalArray<type, _SizeArgs...>;
		using size_type = size_t;
		static constexpr size_type size = _Size1;
	private:
		std::array<child_type, _Size1> m_array;
	public:

		/// <summary>
		/// Get element at index inx
		/// </summary>
		template<typename _InxTy> requires std::convertible_to<_InxTy, size_type>
		inline child_type& operator[](const _InxTy& idx)
		{
			auto inx = static_cast<size_type>(idx);
			if (inx >= size)
				throw e_outOfRange(FORMAT_EXCEPTION_MESSAGE("Index out of range"));
			return m_array[inx];
		}
		template<typename _InxTy> requires std::convertible_to<_InxTy, size_type>
		inline const child_type& operator[](const _InxTy& idx) const
		{
			auto inx = static_cast<size_type>(idx);
			if (inx >= size)
				throw e_outOfRange(FORMAT_EXCEPTION_MESSAGE("Index out of range"));
			return m_array[inx];
		}

		/// <summary>
		/// Fill the array with val.
		/// </summary>
		inline void fill(const type& val)
		{
			for (auto i = 0; i < size; i++)
				m_array[i].fill(val);
		}
	};

	template<typename _Ty, size_t _Size>
	class NDimensionalArray<_Ty, _Size>
	{
		EXCEPTION_CLASS(e_nDimArray, e_lokiError);
		EXCEPTION_CLASS(e_outOfRange, e_nDimArray)
	public:
		using type = std::remove_cvref_t<_Ty>;
		using child_type = type;
		using size_type = size_t;
		static constexpr size_type size = _Size;
	private:
		std::array<type, _Size> m_array;
	public:
		/// <summary>
		/// Get element at index inx
		/// </summary>
		template<typename _InxTy> requires std::convertible_to<_InxTy, size_type>
		inline child_type& operator[](const _InxTy& idx)
		{
			auto inx = static_cast<size_type>(idx);
			if (inx >= size)
				throw e_outOfRange(FORMAT_EXCEPTION_MESSAGE("Index out of range"));
			return m_array[inx];
		}
		template<typename _InxTy> requires std::convertible_to<_InxTy, size_type>
		inline const child_type& operator[](const _InxTy& idx) const
		{
			auto inx = static_cast<size_type>(idx);
			if (inx >= size)
				throw e_outOfRange(FORMAT_EXCEPTION_MESSAGE("Index out of range"));
			return m_array[inx];
		}

		/// <summary>
		/// Fill the array with val.
		/// </summary>
		inline void fill(const type& val)
		{
			m_array.fill(val);
		}
	};
}