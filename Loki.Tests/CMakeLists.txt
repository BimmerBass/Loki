include(FetchContent)

set(CATCH_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
set(CATCH_INSTALL_HELPERS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING ON CACHE BOOL "" FORCE)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.15.0
)
FetchContent_Declare(
  trompeloeil
  GIT_REPOSITORY https://github.com/rollbear/trompeloeil.git
  GIT_TAG v49
)
FetchContent_MakeAvailable(Catch2 trompeloeil)

foreach(catch2_target IN ITEMS Catch2 Catch2WithMain Catch2Extras Catch2WithMainExtras)
  if(TARGET ${catch2_target})
    set_target_properties(${catch2_target} PROPERTIES FOLDER "Dependencies/Catch2")
  endif()
endforeach()

file(GLOB_RECURSE LOKI_TEST_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)
list(FILTER LOKI_TEST_SOURCES EXCLUDE REGEX ".*/pch\\.cpp$")
file(GLOB_RECURSE LOKI_TEST_HEADERS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_executable(loki_tests ${LOKI_TEST_SOURCES} $<TARGET_OBJECTS:loki_core>)
target_sources(loki_tests PRIVATE ${LOKI_TEST_HEADERS})
target_link_libraries(loki_tests PRIVATE Catch2::Catch2WithMain)
target_include_directories(loki_tests
  PRIVATE
    "${LOKI_ROOT_DIR}"
    "${LOKI_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${trompeloeil_SOURCE_DIR}/include"
)

file(TO_CMAKE_PATH "${LOKI_TEST_DIR}/data" LOKI_TEST_DATA_DIR)
target_compile_definitions(loki_tests
  PRIVATE
    DATA_DIR_PATH=\"${LOKI_TEST_DATA_DIR}\"
    $<$<CONFIG:Debug>:LOKI_ENABLE_DEV_COMMANDS>
)

if(MSVC)
  target_compile_options(loki_tests PRIVATE /Zc:preprocessor)
endif()

if(LOKI_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(loki_tests PRIVATE --coverage -O0)
  target_link_options(loki_tests PRIVATE --coverage)
endif()

include(Catch)
# Use PRE_TEST so discovery stays configuration-aware in Visual Studio's
# multi-config CMake integration and Test Explorer can resolve the active build.
catch_discover_tests(loki_tests DISCOVERY_MODE PRE_TEST)

if(LOKI_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  find_program(GCOVR_EXECUTABLE gcovr)
  if(GCOVR_EXECUTABLE)
    add_custom_target(coverage
      COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>
      COMMAND ${GCOVR_EXECUTABLE}
              --root "${LOKI_ROOT_DIR}"
              --exclude "${LOKI_ROOT_DIR}/build/.*"
              --exclude "${LOKI_ROOT_DIR}/Loki.Tests/.*"
              --exclude "${LOKI_ROOT_DIR}/_deps/.*"
      DEPENDS loki_tests
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
      USES_TERMINAL
    )
  endif()
endif()

if(LOKI_ENABLE_COVERAGE AND MSVC)
  find_program(OPENCPCOVERAGE_EXECUTABLE OpenCppCoverage.exe)
  if(OPENCPCOVERAGE_EXECUTABLE)
    add_custom_target(coverage-msvc
      COMMAND ${OPENCPCOVERAGE_EXECUTABLE}
              --export_type cobertura:${CMAKE_BINARY_DIR}/coverage.xml
              --sources "${LOKI_SOURCE_DIR}"
              --sources "${CMAKE_CURRENT_SOURCE_DIR}"
              -- $<TARGET_FILE:loki_tests>
      DEPENDS loki_tests
      USES_TERMINAL
    )
  endif()
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${LOKI_TEST_SOURCES} ${LOKI_TEST_HEADERS})

set_target_properties(loki_tests PROPERTIES FOLDER "Loki")
