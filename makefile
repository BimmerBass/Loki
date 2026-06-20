# Loki, a UCI-compliant chess playing software
# Copyright (C) 2021  Niels Abildskov (https://github.com/BimmerBass)
#
# Loki is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Loki is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

CXX ?= g++
TARGET ?= Loki3

SRC_DIR := Loki
BUILD_DIR ?= build
VERSION_HEADER := $(SRC_DIR)/__gitversion_autogen.hpp

# Build knobs mirrored from the existing configuration and README.
BIT ?= 64
optimize ?= yes
use_popcount ?= yes
perft_transposition_table ?= no
debug ?= no

rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

SOURCES := $(filter-out $(VERSION_HEADER),$(call rwildcard,$(SRC_DIR)/,*.cpp))

OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/obj/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)

CPPFLAGS += -I$(SRC_DIR) -I$(SRC_DIR)/movegen
CXXFLAGS += -std=c++23 -march=native -MMD -MP -pthread
LDFLAGS += -pthread

ifeq ($(optimize),yes)
	CXXFLAGS += -O3
else
	CXXFLAGS += -O0
endif

ifeq ($(debug),yes)
	CXXFLAGS += -g
else
	CXXFLAGS += -DNDEBUG
endif

ifeq ($(use_popcount),yes)
	CXXFLAGS += -DUSE_POPCNT
endif

ifeq ($(perft_transposition_table),yes)
	CXXFLAGS += -DPERFT_TT
endif

ifeq ($(BIT),64)
	CXXFLAGS += -DIS_64BIT -m64
else ifeq ($(BIT),32)
	CXXFLAGS += -m32
else
	$(error BIT must be 32 or 64)
endif

.PHONY: all clean run FORCE

all: $(TARGET)

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(VERSION_HEADER) $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/obj/%.o: $(SRC_DIR)/%.cpp $(VERSION_HEADER)
	@mkdir -p "$(@D)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(VERSION_HEADER): FORCE
	@mkdir -p "$(@D)"
	@commit="$$(git rev-parse --short=8 HEAD 2>/dev/null || echo unknown)"; \
	branch="$$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo unknown)"; \
	printf '%s\n' '// Auto-generated file. Do not edit manually.' > $@; \
	printf '%s\n' '// This file contains the current Git commit and branch information.' >> $@; \
	printf '#define GIT_CUR_COMMIT "%s"\n' "$$commit" >> $@; \
	printf '#define GIT_BRANCH "%s"\n' "$$branch" >> $@;

clean:
	$(RM) -r $(BUILD_DIR)
	$(RM) -f $(VERSION_HEADER)

-include $(DEPS)
