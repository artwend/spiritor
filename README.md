# spiritor

> **Disclaimer:** This project is for educational purposes only.

A Windows utility that reads game log files, watches them for changes, and executes Lua-driven automation scripts by sending keyboard input via the [Interception driver](https://github.com/oblitum/Interception).

## Features

- **Log file watching** — monitors a game's log file and reacts to new entries in real time (`src/FileWatch.h`, `src/log_reader.cpp`).
- **Lua scripting** — automation behavior is defined in Lua scripts (`script/swtor.lua`) executed with Lua + sol2 (`src/script_holder.cpp`).
- **Input injection** — synthesizes keyboard events using the Interception library (`src/interception.c`, `src/input_sender.cpp`).
- **Process tracking** — ETW-based process monitoring to detect when the target game is running (`src/ProcessWatcher_ETW.cpp`, `src/ETW.h`).

## Building

Requirements:

- CMake 3.17+
- A C++20 compiler (MinGW-w64 or MSVC)
- [vcpkg](https://vcpkg.io/) (recommended) for dependencies
- The [Interception driver](https://github.com/oblitum/Interception) installed on the target machine

Dependencies:

- [spdlog](https://github.com/gabime/spdlog)
- [sol2](https://github.com/ThePhD/sol2) and Lua
- Boost (chrono)
- [WIL](https://github.com/microsoft/wil) (Windows Implementation Library)

Configure and build:

```sh
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake
cmake --build cmake-build-release --config Release
```

## Usage

1. Install and activate the Interception driver (see its repository for `install.bat`).
2. Place your automation script in the `script/` directory.
3. Run `spiritor.exe`.

## Project structure

```
src/
  main.cpp               # Entry point and main loop
  log_reader.{h,cpp}     # Game log parsing and reading
  script_holder.{h,cpp}  # Lua script loading and execution
  input_sender.{h,cpp}   # Keyboard event injection
  interception.{h,c}     # Interception driver bindings
  ProcessWatcher_ETW.cpp # ETW-based process monitoring
  parser.h               # Boost.Spirit log grammar
  record.h               # Log record types
script/
  swtor.lua              # Example automation script
```

## License

Distributed under the [MIT License](LICENSE).
