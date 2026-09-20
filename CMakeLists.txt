cmake_minimum_required(VERSION 4.3)
project(csopesy_marquee_console)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(csopesy_marquee_console main.cpp)

# Keep the font assets next to the built binary so the program finds them when
# launched from the build directory (e.g. from CLion).
add_custom_command(TARGET csopesy_marquee_console POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/ascii_art.txt"
            "${CMAKE_CURRENT_SOURCE_DIR}/characters.txt"
            "$<TARGET_FILE_DIR:csopesy_marquee_console>"
    COMMENT "Copying ascii_art.txt and characters.txt next to the executable")
