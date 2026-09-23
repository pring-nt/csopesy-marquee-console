/**
 * @file main.cpp
 * @brief Entry point for the CSOPESY marquee console.
 *
 * @par Course
 *     CSOPESY  ·  Section S01  ·  Group 12
 *
 * @authors
 *     Trinidad, Nathan
 *     Singh, Nathaniel
 *     Quilantang, Jann Miro
 *     Saguin, VL Kirsten Camille
 *
 * Loads the ASCII-art font, then hands control to @ref marquee::Console, which
 * prints the welcome header and runs the command loop that keeps the marquee
 * text, refresh speed, and running flag in memory. The scrolling animation
 * itself is not drawn yet; @c start_marquee and @c stop_marquee only record
 * the requested state through @ref marquee::Marquee.
 *
 * Build (single command, from the repository root):
 *     g++ -std=c++17 -Isrc/include src/main.cpp src/components/console.cpp
 *         src/components/marquee.cpp src/components/font.cpp
 *         src/components/ascii_art.cpp src/components/asset_paths.cpp
 *         src/components/text_utils.cpp -o csopesy.exe
 *
 * Build (CMake):
 *     cmake -S . -B build
 *     cmake --build build
 *
 * The font assets live in @c assets/ and are also copied next to the built
 * binary; @ref marquee::resolveAssetPath accepts either location. Nothing
 * else is needed at runtime.
 */

#include "asset_paths.h"
#include "config.h"
#include "console.h"
#include "font.h"

/**
 * @brief Loads the font and runs the console.
 * @return Zero on a normal exit.
 */
int main() {
    const marquee::Font font =
        marquee::loadFont(marquee::resolveAssetPath(marquee::kFontFile),
                          marquee::resolveAssetPath(marquee::kOrderFile));
    marquee::Console console(font);
    return console.run();
}
