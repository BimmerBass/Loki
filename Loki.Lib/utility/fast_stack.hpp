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
#ifndef FAST_STACK_H
#define FAST_STACK_H

namespace loki::utility
{

	/// <summary>
	/// fast_stack is a stack structure that is allocated on the... stack...
	/// </summary>
	/// <typeparam name="_Ty"></typeparam>
	template<typename _Ty, size_t _Size> requires (_Size > 0)
		class fast_stack
	{
		EXCEPTION_CLASS(e_fastStack, e_lokiError);
	public:
		using value_t = _Ty;
		using reference_t = _Ty&;
		using const_reference_t = const _Ty&;
		static constexpr size_t max_size = _Size;

		using const_iterator_t = std::array<value_t, max_size>::const_iterator;
	protected:
		std::array<value_t, max_size>	m_stack;
		size_t						m_current_size;

	public:
		/// <summary>
		/// Insert a new element to the top of the stack.
		/// </summary>
		/// <param name="element"></param>
		void insert(value_t&& element)
		{
			if (m_current_size >= max_size)
				throw e_fastStack(FORMAT_EXCEPTION_MESSAGE("insert() was attempted on a full fast_stack object."));

			m_stack[m_current_size] = element;
			m_current_size++;
		}

		/// <summary>
		/// Pop the top of the stack.
		/// </summary>
		/// <returns></returns>
		reference_t pop()
		{
			if (m_current_size <= 0)
				throw e_fastStack(FORMAT_EXCEPTION_MESSAGE("pop() was called on an empty fast_stack object."));
			return m_stack[--m_current_size];
		}

		/// <summary>
		/// Get a const reference to the top of the container, but don't actually remove it.
		/// </summary>
		/// <returns></returns>
		const_reference_t top() const
		{
			if (m_current_size <= 0)
				throw e_fastStack(FORMAT_EXCEPTION_MESSAGE("top() was called on an empty fast_stack object."));
			return m_stack[m_current_size - 1];
		}

		/// <summary>
		/// Get the current size of the stack.
		/// </summary>
		/// <returns></returns>
		size_t size() const noexcept
		{
			return m_current_size;
		}

		/// <summary>
		/// Clear the stack.
		/// </summary>
		/// <returns></returns>
		void clear() noexcept
		{
			m_current_size = 0;
		}

		// Default ctor.
		fast_stack() : m_current_size(0), m_stack{}{}

		// copy constructor/operator.
		fast_stack(const fast_stack& _src)
		{
			std::copy(
				std::begin(_src.m_stack),
				std::end(_src.m_stack),
				std::begin(m_stack)
			);
			m_current_size = _src.m_current_size;
		}
		fast_stack& operator=(const fast_stack& _src)
		{
			if (this != &_src)
			{
				std::copy(
					std::begin(_src.m_stack),
					std::end(_src.m_stack),
					std::begin(m_stack)
				);
				m_current_size = _src.m_current_size;
			}
			return *this;
		}

		inline const_iterator_t cbegin() const noexcept { return m_stack.cbegin(); }
		inline const_iterator_t cend() const noexcept { return m_stack.cbegin() + m_current_size; }

		virtual ~fast_stack() {}
	};
}

#endif