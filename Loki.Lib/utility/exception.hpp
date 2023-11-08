#pragma once

// Removes the path to the file that threw the exception.
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define FORMAT_EXCEPTION_MESSAGE(msg, ...) std::format("{} ({}:{})", std::format((msg), __VA_ARGS__), __FILENAME__, __LINE__)

#define EXCEPTION(className)												\
class className : public std::exception {									\
public:																		\
	explicit inline className() : std::exception(#className) {}				\
	explicit inline className(const std::string& desc)						\
		: std::exception(std::format("{}: {}", #className, desc).c_str()) {}\
};

#define EXCEPTION_CLASS(className, superclass)							\
class className : public superclass {									\
public:																	\
	explicit inline className() : 										\
		superclass(#className) {}										\
	explicit inline className(const std::string& desc)					\
		: superclass(std::format("{}: {}", #className, desc).c_str()) {}\
};