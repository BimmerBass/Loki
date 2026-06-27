loki_collect_engine_sources(LOKI_ENGINE_SOURCES LOKI_ENGINE_HEADERS)

set(LOKI_MAIN_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
set(LOKI_VERSION_HEADER "${LOKI_GENERATED_DIR}/versioninfo.hpp")
set(LOKI_GIT_VERSION_HEADER "${LOKI_GENERATED_DIR}/__gitversion_autogen.hpp")

set_source_files_properties(
  "${LOKI_VERSION_HEADER}"
  "${LOKI_GIT_VERSION_HEADER}"
  PROPERTIES GENERATED TRUE
)

add_executable(Loki
  "${LOKI_MAIN_SOURCE}"
  ${LOKI_ENGINE_SOURCES}
  ${LOKI_ENGINE_HEADERS}
  "${LOKI_VERSION_HEADER}"
  "${LOKI_GIT_VERSION_HEADER}"
)

add_dependencies(Loki Loki.GitVersion)

loki_apply_common_target_options(Loki)
loki_apply_release_optimizations(Loki)

if(WIN32)
  set(LOKI_ICON_SOURCE "${LOKI_ROOT_DIR}/Icon.png")
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

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES
  "${LOKI_MAIN_SOURCE}"
  ${LOKI_ENGINE_SOURCES}
  ${LOKI_ENGINE_HEADERS}
)
source_group("Generated" FILES "${LOKI_VERSION_HEADER}" "${LOKI_GIT_VERSION_HEADER}")

set_target_properties(Loki PROPERTIES FOLDER "Loki")
