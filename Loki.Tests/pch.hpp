//
// pch.h
//

#pragma once

#include "gtest/gtest.h"
#include "gmock/gmock.h"

// STL includes
#include <filesystem>
#include <fstream>
#include <ranges>
#include <tuple>

#define STRINGIFY(x) #x
#define EXPAND(x) STRINGIFY(x)

inline std::string get_project_dir()
{
	std::string s = EXPAND(DATA_DIR_PATH);
	return s.erase(0,1).erase(s.size() - 1);
}

// For an introduction on google test: https://github.com/google/googletest/blob/main/docs/primer.md
