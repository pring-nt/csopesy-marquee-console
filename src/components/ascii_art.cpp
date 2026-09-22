/**
 * @file ascii_art.cpp
 * @brief Implementation of the ASCII-art renderer.
 *
 * @par Course
 *     CSOPESY  ·  Section S01  ·  Group 12
 *
 * @authors
 *     Trinidad, Nathan
 *     Singh, Nathaniel
 *     Quilantang, Jann Miro
 *     Saguin, VL Kirsten Camille
 */

#include "ascii_art.h"

#include <cstdlib>
#include <istream>
#include <ostream>
#include <sstream>

#include "config.h"
#include "text_utils.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace marquee {

int getDisplayWidth() {
    constexpr int kFallback = 80;
    if (const char* columns = std::getenv("COLUMNS")) {
        std::istringstream stream(columns);
        long long value = 0;
        stream >> value;
        if (!stream.fail()) {
            // Consume any trailing whitespace, then require end of input.
            // Test eof() only: newer libstdc++ makes std::ws set failbit once
            // it reaches EOF, so also requiring !fail() would reject every
            // valid value and silently ignore the COLUMNS setting.
            stream >> std::ws;
            if (stream.eof() && value >= 20 && value <= 1000) {
                return static_cast<int>(value);
            }
        }
    }
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        const int width = info.srWindow.Right - info.srWindow.Left + 1;
        if (width >= 20 && width <= 1000) {
            return width;
        }
    }
#else
    struct winsize size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0 && size.ws_col > 0) {
        return static_cast<int>(size.ws_col);
    }
#endif
    return kFallback;
}

std::vector<std::string> renderAsciiArt(const std::string& text,
                                        const Font& font,
                                        int maxWidth) {
    std::vector<std::string> rows;
    if (!font.valid()) {
        return rows;
    }
    const std::string upper = toUpper(text);
    const std::string gap(static_cast<std::string::size_type>(kGlyphGap), ' ');
    rows.reserve(static_cast<std::vector<std::string>::size_type>(font.height));
    for (int row = 0; row < font.height; ++row) {
        std::string composite;
        for (std::string::size_type i = 0; i < upper.size(); ++i) {
            composite += lookupGlyph(font, upper[i])[row];
            composite += gap;
        }
        if (maxWidth >= 0 && static_cast<int>(composite.size()) > maxWidth) {
            composite.resize(static_cast<std::string::size_type>(maxWidth));
        }
        rows.push_back(rightTrim(composite));
    }
    return rows;
}

void printAsciiArt(const std::string& text, const Font& font, std::ostream& out) {
    const std::vector<std::string> rows =
        renderAsciiArt(text, font, getDisplayWidth());
    for (std::vector<std::string>::size_type i = 0; i < rows.size(); ++i) {
        out << rows[i] << '\n';
    }
}

}  // namespace marquee
