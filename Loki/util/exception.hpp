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
#include <format>
#include <stacktrace>
#include <stdexcept>

#define BASE_EXCEPTION(className)					                        \
class className : public ::loki::stacktrace_exception {			            \
public:												                        \
    static constexpr size_t SKIP_FRAME_COUNT =                              \
        ::loki::stacktrace_exception::SKIP_FRAME_COUNT + 1;                 \
    explicit inline className(size_t skip_count = SKIP_FRAME_COUNT)         \
        : ::loki::stacktrace_exception(#className, skip_count) {}           \
    explicit inline className(                                              \
        const char* msg, size_t skip_count = SKIP_FRAME_COUNT)              \
        : ::loki::stacktrace_exception(                                     \
            std::format("{}: {}", #className, msg).c_str(),                 \
            skip_count) {}                                                  \
};

#define CHILD_EXCEPTION(className, superclass)                              \
class className : public superclass {                                       \
public:                                                                     \
    static constexpr size_t SKIP_FRAME_COUNT =                              \
        superclass::SKIP_FRAME_COUNT + 1;                                   \
    explicit inline className(size_t skip_frames = SKIP_FRAME_COUNT)        \
        : superclass(#className, skip_frames) {}                            \
    explicit inline className(                                              \
        const char* msg, size_t skip_frames = SKIP_FRAME_COUNT)             \
        : superclass(std::format("{}: {}", #className, msg).c_str(),        \
                     skip_frames) {}                                        \
};

namespace loki
{
    class stacktrace_exception : public std::exception
    {
    public:
        static constexpr size_t SKIP_FRAME_COUNT = 1;

        explicit inline stacktrace_exception(const char* msg, size_t skip_frames = SKIP_FRAME_COUNT) : std::exception(msg)
        {
            m_stacktrace = std::stacktrace::current(skip_frames);
        }

        // one method wich has both value, const and rvalue overloads using "deducing this"
        // https://devblogs.microsoft.com/cppblog/cpp23-deducing-this/
        template<class Self>
        auto&& trace(this Self&& self)
        {
            return std::forward<Self>(self).m_stacktrace;
        }

    private:
        std::stacktrace m_stacktrace;
    };

    /// <summary>
    /// Loki's base exception class.
    /// All Loki-exceptions should ultimately derive from this.
    /// </summary>
    BASE_EXCEPTION(loki_exception);

    /// <summary>
    /// Used for intentionally non-implemented features.
    /// </summary>
    CHILD_EXCEPTION(not_implemented_error, loki_exception);


    /// <summary>
    /// Utility function that throws an exception with a formatted message string.
    /// </summary>
    /// <typeparam name="ExceptionType">The type of exception to throw</typeparam>
    /// <typeparam name="...Args">Arguments types for std::format</typeparam>
    /// <param name="format_str">Format string</param>
    /// <param name="...args">Variables to pass to std::format</param>
    template<typename ExceptionType, typename... Args>
    [[noreturn]] void throw_msg(const std::format_string<Args...>& format_str, Args&&... args)
    {
        std::string message = std::format(format_str, std::forward<Args>(args)...);
        throw ExceptionType(message.c_str(), ExceptionType::SKIP_FRAME_COUNT + 1);
    }
}