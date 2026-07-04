# Building Loki
Loki uses CMake as its only supported build system. The checked-in presets currently target Visual Studio 18 2026 on x64.

## Requirements
- CMake 3.24 or newer.
- A C++23 compliant compiler.
- Visual Studio 18 2026 for the checked-in presets.

The top-level CMake project is currently versioned as `4.0.0`.

## Presets
Configure, build, and test Debug:

```powershell
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Configure, build, and test Release:

```powershell
cmake --preset release
cmake --build --preset release
ctest --preset release
```

The presets write build output under `build/debug` and `build/release`. Executables are placed in each build tree's `bin` directory.

## Dev commands
The `LOKI_DEV_COMMANDS` cache variable controls extra UCI commands used for development:
- `AUTO` enables dev commands in Debug builds and disables them in Release builds.
- `ON` always enables dev commands.
- `OFF` always disables dev commands.

For example, to build a Release binary with perft and other dev commands enabled:

```powershell
cmake --preset release -DLOKI_DEV_COMMANDS=ON
cmake --build --preset release
```

## Dependencies
CMake fetches the test dependencies automatically through `FetchContent`:
- Catch2 `v3.15.0`
- trompeloeil `v49`

After the initial fetch, `FETCHCONTENT_UPDATES_DISCONNECTED` is enabled so dependencies are not updated automatically on every configure.

## CMake structure
The build is split into three CMake entry points:
- [`../CMakeLists.txt`](../CMakeLists.txt) defines shared configuration, project metadata, dependency fetching, generated version headers, and common target options.
- [`../Loki/CMakeLists.txt`](../Loki/CMakeLists.txt) defines the `Loki` executable.
- [`../Loki.Tests/CMakeLists.txt`](../Loki.Tests/CMakeLists.txt) defines the `Loki.Tests` executable and Catch2 test discovery.

The active test suite targets code under `Loki/`. `Loki.Deprecated/` remains reference material only and is intentionally excluded from the current build.
