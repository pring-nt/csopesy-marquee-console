# 3.10 is the floor: old enough for the CMake bundled with older CLion
# versions, and still accepted by CMake 4.x, which dropped compatibility with
# anything below 3.5.
cmake_minimum_required(VERSION 3.10)
project(csopesy_marquee_console)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Module headers live in src/include/ and their implementations in
# src/components/; src/main.cpp is the entry point. See src/include/*.h for
# the interface each module exposes.
add_executable(csopesy_marquee_console
    src/main.cpp
    src/components/ascii_art.cpp
    src/components/asset_paths.cpp
    src/components/console.cpp
    src/components/font.cpp
    src/components/marquee.cpp
    src/components/terminal.cpp
    src/components/text_utils.cpp
)

target_include_directories(csopesy_marquee_console PRIVATE src/include)

# The marquee animation runs on its own thread, so the thread library has to be
# linked explicitly (-pthread on Linux/macOS; a no-op on MinGW).
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(csopesy_marquee_console PRIVATE Threads::Threads)

# Keep the font assets next to the built binary as well as in assets/, so the
# program finds them whether it is launched from the build directory (e.g. from
# CLion) or from the repository root.
add_custom_command(TARGET csopesy_marquee_console POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/ascii_art.txt"
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/characters.txt"
            "$<TARGET_FILE_DIR:csopesy_marquee_console>"
    COMMENT "Copying ascii_art.txt and characters.txt next to the executable")
