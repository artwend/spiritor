cmake_minimum_required(VERSION 3.17)
project(spiritor)

set(CMAKE_CXX_STANDARD 20)
#set(VCPKG_APPLOCAL_DEPS OFF)

add_executable(spiritor
        src/main.cpp
        src/log_reader.cpp
        src/script_holder.cpp
        src/interception.c
)

include(FindLua)
find_package(Lua)
find_package(spdlog CONFIG REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_path(WIL_INCLUDE_DIRS "wil/com.h")

target_compile_definitions(spiritor PRIVATE BOOST_SPIRIT_UNICODE)
target_include_directories(spiritor PRIVATE ${WIL_INCLUDE_DIRS})# ${LUA_INCLUDE_DIR})
target_link_libraries(spiritor
        PRIVATE
        spdlog::spdlog
        ${LUA_LIBRARIES}
        sol2::sol2
        )