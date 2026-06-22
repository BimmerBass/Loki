file(GLOB_RECURSE LOKI_CORE_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)
list(REMOVE_ITEM LOKI_CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

add_library(loki_core OBJECT ${LOKI_CORE_SOURCES})
target_include_directories(loki_core
  PUBLIC
    "${LOKI_ROOT_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
  PRIVATE
    "${LOKI_GENERATED_DIR}"
)
target_compile_features(loki_core PUBLIC cxx_std_23)
target_compile_definitions(loki_core
  PRIVATE
    $<$<CONFIG:Debug>:LOKI_ENABLE_DEV_COMMANDS>
)

add_executable(Loki "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp" $<TARGET_OBJECTS:loki_core>)
target_include_directories(Loki PRIVATE "${LOKI_ROOT_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" "${LOKI_GENERATED_DIR}")
target_compile_features(Loki PRIVATE cxx_std_23)

if(LOKI_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(loki_core PRIVATE --coverage -O0)
  target_compile_options(Loki PRIVATE --coverage -O0)
  target_link_options(Loki PRIVATE --coverage)
endif()

set_target_properties(loki_core PROPERTIES FOLDER "Loki/Core")
set_target_properties(Loki PROPERTIES FOLDER "Loki")
