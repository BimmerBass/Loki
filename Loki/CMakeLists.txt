file(GLOB_RECURSE LOKI_CORE_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)
list(REMOVE_ITEM LOKI_CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
file(GLOB_RECURSE LOKI_CORE_HEADERS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

include(GNUInstallDirs)

set(LOKI_ICON_SOURCE "${LOKI_ROOT_DIR}/Icon.png")

add_library(loki_core OBJECT ${LOKI_CORE_SOURCES})
target_sources(loki_core PRIVATE ${LOKI_CORE_HEADERS})
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

if(WIN32)
  set(LOKI_WIN_ICON "${LOKI_GENERATED_DIR}/Loki.ico")
  set(LOKI_WIN_ICON_RC "${LOKI_GENERATED_DIR}/LokiIcon.rc")

  add_custom_command(
    OUTPUT "${LOKI_WIN_ICON}"
    COMMAND powershell -NoProfile -ExecutionPolicy Bypass
            -File "${LOKI_ROOT_DIR}/cmake/create_loki_windows_icon.ps1"
            -InputPng "${LOKI_ICON_SOURCE}"
            -OutputIco "${LOKI_WIN_ICON}"
    DEPENDS "${LOKI_ICON_SOURCE}" "${LOKI_ROOT_DIR}/cmake/create_loki_windows_icon.ps1"
    VERBATIM
  )

  configure_file(
    "${LOKI_ROOT_DIR}/cmake/LokiIcon.rc.in"
    "${LOKI_WIN_ICON_RC}"
    @ONLY
  )
  set_source_files_properties("${LOKI_WIN_ICON_RC}" PROPERTIES OBJECT_DEPENDS "${LOKI_WIN_ICON}")
  target_sources(Loki PRIVATE "${LOKI_WIN_ICON_RC}")
endif()

if(LOKI_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(loki_core PRIVATE --coverage -O0)
  target_compile_options(Loki PRIVATE --coverage -O0)
  target_link_options(Loki PRIVATE --coverage)
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${LOKI_CORE_SOURCES} ${LOKI_CORE_HEADERS})

install(TARGETS Loki RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

if(UNIX AND NOT APPLE)
  configure_file(
    "${LOKI_ROOT_DIR}/cmake/loki.desktop.in"
    "${LOKI_GENERATED_DIR}/loki.desktop"
    @ONLY
  )
  install(FILES "${LOKI_ICON_SOURCE}" DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps" RENAME loki.png)
  install(FILES "${LOKI_GENERATED_DIR}/loki.desktop" DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
endif()

set_target_properties(loki_core PROPERTIES FOLDER "Loki")
set_target_properties(Loki PROPERTIES FOLDER "Loki")
