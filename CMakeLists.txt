cmake_minimum_required(VERSION 3.24)

project(Loki
  VERSION 4.0.0
  DESCRIPTION "Loki C++23 Chess Engine"
  LANGUAGES CXX
)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not allowed. Use a preset, e.g. cmake --preset debug")
endif()

include(FetchContent)
include(CheckIPOSupported)

enable_testing()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LOKI_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(LOKI_SOURCE_DIR "${LOKI_ROOT_DIR}/Loki")
set(LOKI_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

file(MAKE_DIRECTORY "${LOKI_GENERATED_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

foreach(loki_config IN ITEMS Debug Release)
  string(TOUPPER "${loki_config}" loki_config_upper)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${loki_config_upper} "${CMAKE_BINARY_DIR}/bin")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${loki_config_upper} "${CMAKE_BINARY_DIR}/lib")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${loki_config_upper} "${CMAKE_BINARY_DIR}/lib")
endforeach()

set(LOKI_DEV_COMMANDS "AUTO" CACHE STRING "Enable dev UCI commands: AUTO, ON, or OFF")
set_property(CACHE LOKI_DEV_COMMANDS PROPERTY STRINGS AUTO ON OFF)
string(TOUPPER "${LOKI_DEV_COMMANDS}" LOKI_DEV_COMMANDS_NORMALIZED)
set(LOKI_DEV_COMMANDS_VALUES AUTO ON OFF)
if(NOT LOKI_DEV_COMMANDS_NORMALIZED IN_LIST LOKI_DEV_COMMANDS_VALUES)
  message(FATAL_ERROR "LOKI_DEV_COMMANDS must be AUTO, ON, or OFF")
endif()
set(LOKI_DEV_COMMANDS "${LOKI_DEV_COMMANDS_NORMALIZED}" CACHE STRING "Enable dev UCI commands: AUTO, ON, or OFF" FORCE)

check_ipo_supported(RESULT LOKI_IPO_SUPPORTED OUTPUT LOKI_IPO_OUTPUT)

configure_file(
  "${LOKI_SOURCE_DIR}/versioninfo.hpp.in"
  "${LOKI_GENERATED_DIR}/versioninfo.hpp"
  @ONLY
)

add_custom_target(Loki.GitVersion ALL
  COMMAND "${CMAKE_COMMAND}"
          "-DLOKI_ROOT_DIR=${LOKI_ROOT_DIR}"
          "-DLOKI_GIT_VERSION_HEADER=${LOKI_GENERATED_DIR}/__gitversion_autogen.hpp"
          -P "${LOKI_ROOT_DIR}/cmake/write_git_version.cmake"
  BYPRODUCTS "${LOKI_GENERATED_DIR}/__gitversion_autogen.hpp"
  COMMENT "Generating Git version header"
  VERBATIM
)
set_target_properties(Loki.GitVersion PROPERTIES FOLDER "Loki")

function(loki_collect_engine_sources out_sources out_headers)
  file(GLOB_RECURSE loki_sources CONFIGURE_DEPENDS
    "${LOKI_SOURCE_DIR}/*.cpp"
  )
  file(GLOB_RECURSE loki_headers CONFIGURE_DEPENDS
    "${LOKI_SOURCE_DIR}/*.h"
    "${LOKI_SOURCE_DIR}/*.hpp"
  )

  list(REMOVE_ITEM loki_sources "${LOKI_SOURCE_DIR}/main.cpp")
  list(REMOVE_ITEM loki_headers "${LOKI_SOURCE_DIR}/__gitversion_autogen.hpp")

  set(${out_sources} ${loki_sources} PARENT_SCOPE)
  set(${out_headers} ${loki_headers} PARENT_SCOPE)
endfunction()

function(loki_apply_common_target_options target_name)
  target_compile_features(${target_name} PRIVATE cxx_std_23)
  target_include_directories(${target_name}
    PRIVATE
      "${LOKI_GENERATED_DIR}"
      "${LOKI_ROOT_DIR}"
      "${LOKI_SOURCE_DIR}"
  )
  target_compile_definitions(${target_name}
    PRIVATE
      "$<$<OR:$<STREQUAL:${LOKI_DEV_COMMANDS},ON>,$<AND:$<STREQUAL:${LOKI_DEV_COMMANDS},AUTO>,$<CONFIG:Debug>>>:LOKI_ENABLE_DEV_COMMANDS>"
  )

  if(MSVC)
    target_compile_options(${target_name} PRIVATE /EHsc /Zc:preprocessor)
  endif()
endfunction()

function(loki_apply_release_optimizations target_name)
  if(LOKI_IPO_SUPPORTED)
    set_property(TARGET ${target_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
  endif()

  if(MSVC)
    target_compile_options(${target_name}
      PRIVATE
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/Ob3>
        $<$<CONFIG:Release>:/Oi>
        $<$<CONFIG:Release>:/Ot>
        $<$<CONFIG:Release>:/GL>
        $<$<CONFIG:Release>:/fp:fast>
        $<$<CONFIG:Release>:/arch:AVX2>
        $<$<CONFIG:Release>:/GS->
    )
    target_link_options(${target_name}
      PRIVATE
        $<$<CONFIG:Release>:/LTCG>
        $<$<CONFIG:Release>:/OPT:REF>
        $<$<CONFIG:Release>:/OPT:ICF>
    )
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target_name}
      PRIVATE
        $<$<CONFIG:Release>:-Ofast>
        $<$<CONFIG:Release>:-march=native>
        $<$<CONFIG:Release>:-funroll-loops>
        $<$<CONFIG:Release>:-fno-stack-protector>
    )
  endif()
endfunction()

set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Do not update fetched dependencies after the initial fetch")
set(CATCH_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "" FORCE)
set(CATCH_INSTALL_HELPERS OFF CACHE BOOL "" FORCE)
set(TROMPELOEIL_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(TROMPELOEIL_INSTALL_TARGETS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.15.0
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)
FetchContent_Declare(
  trompeloeil
  GIT_REPOSITORY https://github.com/rollbear/trompeloeil.git
  GIT_TAG v49
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(Catch2 trompeloeil)

list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")

foreach(catch2_target IN ITEMS Catch2 Catch2WithMain Catch2Extras Catch2WithMainExtras)
  if(TARGET ${catch2_target})
    set_target_properties(${catch2_target} PROPERTIES FOLDER "Dependencies/Catch2")
  endif()
endforeach()

add_subdirectory(Loki)
add_subdirectory(Loki.Tests)

if(CMAKE_GENERATOR MATCHES "Visual Studio")
  set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Loki)
endif()
