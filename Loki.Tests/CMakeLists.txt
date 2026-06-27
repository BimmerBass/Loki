loki_collect_engine_sources(LOKI_ENGINE_SOURCES LOKI_ENGINE_HEADERS)

file(GLOB_RECURSE LOKI_TEST_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)
file(GLOB_RECURSE LOKI_TEST_HEADERS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

set(LOKI_VERSION_HEADER "${LOKI_GENERATED_DIR}/versioninfo.hpp")
set(LOKI_GIT_VERSION_HEADER "${LOKI_GENERATED_DIR}/__gitversion_autogen.hpp")
set_source_files_properties(
  "${LOKI_VERSION_HEADER}"
  "${LOKI_GIT_VERSION_HEADER}"
  PROPERTIES GENERATED TRUE
)

add_executable(Loki.Tests
  ${LOKI_TEST_SOURCES}
  ${LOKI_TEST_HEADERS}
  ${LOKI_ENGINE_SOURCES}
  ${LOKI_ENGINE_HEADERS}
  "${LOKI_VERSION_HEADER}"
  "${LOKI_GIT_VERSION_HEADER}"
)

add_dependencies(Loki.Tests Loki.GitVersion)

loki_apply_common_target_options(Loki.Tests)
loki_apply_release_optimizations(Loki.Tests)

target_include_directories(Loki.Tests
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(Loki.Tests PRIVATE Catch2::Catch2WithMain)

if(TARGET trompeloeil::trompeloeil)
  target_link_libraries(Loki.Tests PRIVATE trompeloeil::trompeloeil)
elseif(TARGET trompeloeil)
  target_link_libraries(Loki.Tests PRIVATE trompeloeil)
else()
  target_include_directories(Loki.Tests PRIVATE "${trompeloeil_SOURCE_DIR}/include")
endif()

file(TO_CMAKE_PATH "${LOKI_ROOT_DIR}/tests" LOKI_TEST_DATA_DIR)
target_compile_definitions(Loki.Tests PRIVATE DATA_DIR_PATH=\"${LOKI_TEST_DATA_DIR}\")

include(Catch)
catch_discover_tests(Loki.Tests DISCOVERY_MODE PRE_TEST)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Tests" FILES
  ${LOKI_TEST_SOURCES}
  ${LOKI_TEST_HEADERS}
)
source_group(TREE "${LOKI_SOURCE_DIR}" PREFIX "Engine" FILES
  ${LOKI_ENGINE_SOURCES}
  ${LOKI_ENGINE_HEADERS}
)
source_group("Generated" FILES "${LOKI_VERSION_HEADER}" "${LOKI_GIT_VERSION_HEADER}")

set_target_properties(Loki.Tests PROPERTIES FOLDER "Loki")
