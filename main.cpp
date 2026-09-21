/**
 * @file main.cpp
 * @brief Interactive console shell for the CSOPESY marquee.
 *
 * Reads an ASCII-art font from @c ascii_art.txt and the glyph order from
 * @c characters.txt, prints a welcome header, and then runs a command loop
 * that keeps the marquee text, refresh speed, and running flag in memory. The
 * scrolling animation itself is not drawn; @c start_marquee and
 * @c stop_marquee only record the requested state.
 *
 * Build:
 *     g++ -std=c++17 main.cpp -o csopesy
 *
 * Both @c ascii_art.txt and @c characters.txt must sit in the working
 * directory next to the executable.
 */

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace {

/// Path of the file holding the raw glyph blocks, top to bottom.
const std::string kFontFile = "ascii_art.txt";

/// Path of the file listing the glyph characters, one per line.
const std::string kOrderFile = "characters.txt";

/// Text shown in the header and used until @c set_text replaces it.
const std::string kDefaultMarqueeText = "CSOPESY";

/// Refresh interval, in milliseconds, used until @c set_speed replaces it.
constexpr int kDefaultMarqueeSpeed = 200;

/// Date printed in the welcome header.
const std::string kVersionDate = "2026-09-18";

/// Developer names printed in the welcome header.
const std::vector<std::string> kDevelopers = {
    "Trinidad, Nathan",
    "Singh, Nathaniel",
    "Quilantang, Jann Miro",
    "Saguin, Kirsten Camille",
};

/// Columns of separation inserted between adjacent glyphs when rendering.
/// Glyphs are stored at their natural ink width (trailing padding stripped),
/// so this small fixed gap replaces the dead space the old global-width
/// padding produced around narrow glyphs such as 'I' or '!'.
constexpr int kGlyphGap = 1;

/// Width of the blank block used for spaces and unknown characters.
constexpr int kBlankWidth = 4;

/// One glyph: its rows, top row first.
using Glyph = std::vector<std::string>;

/// Maps each supported character to its glyph.
using FontMap = std::map<char, Glyph>;

/// A parsed font ready for rendering.
struct Font {
    int height = 0;        ///< Number of rows in every glyph.
    int width = 0;         ///< Ink width of the widest glyph.
    FontMap glyphs;        ///< Glyph looked up by character.
    Glyph blank;           ///< Blank block used for spaces and unknown characters.
};

/// Marquee settings carried across commands.
struct MarqueeState {
    std::string text = kDefaultMarqueeText;  ///< Current marquee text.
    int speed = kDefaultMarqueeSpeed;        ///< Refresh interval in milliseconds.
    bool running = false;                    ///< Whether the marquee is active.
};

/**
 * @brief Removes leading and trailing whitespace.
 * @param text String to trim.
 * @return A copy of @p text without surrounding spaces, tabs, or newlines.
 */
std::string trim(const std::string& text) {
    const char* kWhitespace = " \t\r\n";
    const std::string::size_type first = text.find_first_not_of(kWhitespace);
    if (first == std::string::npos) {
        return "";
    }
    const std::string::size_type last = text.find_last_not_of(kWhitespace);
    return text.substr(first, last - first + 1);
}

/**
 * @brief Converts every character of @p text to uppercase.
 * @param text String to convert.
 * @return The uppercased copy, leaving non-letters unchanged.
 */
std::string toUpper(const std::string& text) {
    std::string result = text;
    for (char& ch : result) {
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
    return result;
}

/**
 * @brief Removes trailing spaces from @p text.
 * @param text String to trim.
 * @return The string with any trailing spaces removed.
 */
std::string rightTrim(const std::string& text) {
    const std::string::size_type last = text.find_last_not_of(' ');
    if (last == std::string::npos) {
        return "";
    }
    return text.substr(0, last + 1);
}

/**
 * @brief Reads a text file into a vector of lines.
 *
 * Carriage returns are stripped so files with Windows line endings load the
 * same as files with Unix line endings. A file that cannot be opened produces
 * a warning on standard error.
 *
 * @param path File to read.
 * @return One entry per line, or an empty vector if the file cannot be opened.
 */
std::vector<std::string> readLines(const std::string& path) {
    std::vector<std::string> lines;
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Warning: could not open " << path
                  << " - falling back to blank glyphs.\n";
        return lines;
    }
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    return lines;
}

/**
 * @brief Builds the font from the glyph and order files.
 *
 * The glyph order file lists one character per line; a blank line stands for a
 * space. The art file stores the matching glyph blocks in the same order, so
 * the glyph height is the number of art lines divided by the number of
 * characters, and the glyph ink width is the widest ink row found. Each
 * glyph keeps only its natural ink width (trailing padding stripped) so
 * narrow glyphs are not stretched to the width of the widest one (e.g.
 * 'W'); a single-column gap inserted at render time keeps characters
 * separated.
 *
 * @param fontPath File containing the raw glyph blocks.
 * @param orderPath File listing one character per glyph.
 * @return The parsed font, or a font with no glyphs if either file is unusable.
 */
Font loadFont(const std::string& fontPath, const std::string& orderPath) {
    Font font;

    const std::vector<std::string> orderLines = readLines(orderPath);
    const std::vector<std::string> artLines = readLines(fontPath);

    std::vector<char> order;
    order.reserve(orderLines.size());
    for (const std::string& line : orderLines) {
        order.push_back(line.empty() ? ' ' : line.front());
    }

    if (order.empty() || artLines.empty()) {
        return {};
    }

    const auto height = artLines.size() / order.size();
    if (height == 0) {
        std::cerr << "Warning: " << fontPath
                  << " does not contain one glyph per character listed in "
                  << orderPath << ".\n";
        return {};
    }
    if (artLines.size() % order.size() != 0) {
        std::cerr << "Warning: " << fontPath << " has " << artLines.size()
                  << " lines for " << order.size()
                  << " characters - expected a multiple of " << order.size()
                  << " (" << order.size() * height << " lines); ignoring "
                  << (artLines.size() % order.size())
                  << " trailing line(s).\n";
    }
    font.height = static_cast<int>(height);

    for (const std::string& line : artLines) {
        font.width = std::max(font.width,
                               static_cast<int>(rightTrim(line).size()));
    }
    font.blank = Glyph(font.height, std::string(kBlankWidth, ' '));

    for (std::string::size_type i = 0; i < order.size(); ++i) {
        std::string::size_type natural = 0;
        for (std::string::size_type row = 0; row < height; ++row) {
            natural = std::max(
                natural, rightTrim(artLines[i * height + row]).size());
        }
        if (natural == 0) {
            // All-blank block (e.g. the explicit space glyph): keep the
            // standard blank width instead of collapsing to one column.
            natural = static_cast<std::string::size_type>(kBlankWidth);
        }
        Glyph glyph;
        glyph.reserve(height);
        for (std::string::size_type row = 0; row < height; ++row) {
            std::string text = rightTrim(artLines[i * height + row]);
            text.resize(natural, ' ');
            glyph.push_back(text);
        }
        font.glyphs[order[i]] = glyph;
    }
    return font;
}

/**
 * @brief Looks up the glyph for a character.
 * @param font Font to search.
 * @param ch Character to render.
 * @return The character's glyph, or the blank block when it is not mapped.
 */
const Glyph& lookupGlyph(const Font& font, char ch) {
    const auto it = font.glyphs.find(ch);
    if (it != font.glyphs.end() && !it->second.empty()) {
        return it->second;
    }
    return font.blank;
}

/**
 * @brief Returns the visible console width in columns.
 *
 * Long ASCII-art rows wider than the console would be hard-wrapped by the
 * terminal mid-row, spilling glyph fragments onto the next visual line and
 * corrupting the block alignment. The width is used to cut off overflowing
 * art rows (the scrolling marquee reveals the hidden tail). The @c COLUMNS environment
 * variable wins when valid (useful for tests/redirection), then the live
 * console size is queried, otherwise 80 columns is assumed.
 *
 * @return Console width in columns, always positive.
 */
int getDisplayWidth() {
    constexpr int kFallback = 80;
    if (const char* columns = std::getenv("COLUMNS")) {
        std::istringstream stream(columns);
        long long value = 0;
        stream >> value;
        stream >> std::ws;
        if (!stream.fail() && stream.eof() && value >= 20 && value <= 1000) {
            return static_cast<int>(value);
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

/**
 * @brief Renders @p text as ASCII art.
 *
 * The text is uppercased, each character is replaced by its natural-width
 * glyph, and the glyphs are concatenated row by row with a single-column
 * gap and printed. Any composite row wider than the console is cut off
 * instead of wrapped: the marquee scrolls, so the hidden tail is revealed
 * by the animation rather than spilling onto extra lines and corrupting the
 * block layout. Trailing spaces are removed from each printed row.
 *
 * @param text Text to render.
 * @param font Font used for the glyphs.
 */
void printAsciiArt(const std::string& text, const Font& font) {
    if (font.height <= 0 || font.width <= 0) {
        return;
    }
    const std::string upper = toUpper(text);
    const int displayWidth = getDisplayWidth();
    const std::string gap(static_cast<std::string::size_type>(kGlyphGap), ' ');
    for (int row = 0; row < font.height; ++row) {
        std::string composite;
        for (char ch : upper) {
            composite += lookupGlyph(font, ch)[row];
            composite += gap;
        }
        if (static_cast<int>(composite.size()) > displayWidth) {
            composite.resize(
                static_cast<std::string::size_type>(displayWidth));
        }
        std::cout << rightTrim(composite) << '\n';
    }
}

/**
 * @brief Prints the welcome header: the title art and the metadata block.
 * @param font Font used to render the title.
 */
void printHeader(const Font& font) {
    printAsciiArt(kDefaultMarqueeText, font);
    std::cout << '\n';
    std::cout << "Group developer:\n";
    for (const std::string& developer : kDevelopers) {
        std::cout << developer << '\n';
    }
    std::cout << "\nVersion date: " << kVersionDate << "\n\n";
}

/// Prints the list of commands and their descriptions.
void printHelp() {
    std::cout
        << "help            - displays the commands and its description\n"
        << "start_marquee   - starts the marquee \"animation\"\n"
        << "stop_marquee    - stops the marquee \"animation\"\n"
        << "set_text        - accepts a text input and displays it as a marquee\n"
        << "set_speed       - sets the marquee animation refresh in milliseconds\n"
        << "exit            - terminates the console\n";
}

/**
 * @brief Parses a strictly positive integer.
 *
 * The whole string must be one number: leading and trailing whitespace is
 * ignored, but any other trailing character makes the parse fail.
 *
 * @param text String to parse.
 * @param[out] out Receives the parsed value on success.
 * @return @c true when @p text holds a positive integer, @c false otherwise.
 */
bool parsePositiveInt(const std::string& text, int& out) {
    if (text.empty()) {
        return false;
    }
    std::istringstream stream(text);
    long long value = 0;
    stream >> value;
    if (stream.fail()) {
        return false;
    }
    stream >> std::ws;
    if (!stream.eof()) {
        return false;  // trailing characters are not allowed
    }
    if (value <= 0 || value > std::numeric_limits<int>::max()) {
        return false;
    }
    out = static_cast<int>(value);
    return true;
}

}  // namespace

/**
 * @brief Loads the font, prints the header, and runs the command loop.
 * @return Zero on a normal exit.
 */
int main() {
    const Font font = loadFont(kFontFile, kOrderFile);
    printHeader(font);

    MarqueeState state;
    std::string input;

    while (true) {
        std::cout << "Command> " << std::flush;
        if (!std::getline(std::cin, input)) {
            std::cout << "\nTerminating console...\n";
            return 0;
        }

        const std::string line = trim(input);
        std::istringstream stream(line);
        std::string command;
        stream >> command;
        std::string args;
        std::getline(stream, args);
        args = trim(args);

        if (command == "help") {
            printHelp();
        } else if (command == "set_text") {
            if (args.empty()) {
                std::cout
                    << "Error: set_text requires a non-empty string argument.\n";
            } else {
                state.text = args;
                std::cout << "Text saved for marquee: " << state.text << '\n';
                printAsciiArt(state.text, font);
            }
        } else if (command == "start_marquee") {
            state.running = true;
            std::cout
                << "Marquee started. (Animation stub - not yet implemented)\n";
        } else if (command == "stop_marquee") {
            state.running = false;
            std::cout
                << "Marquee stopped. (Animation stub - not yet implemented)\n";
        } else if (command == "set_speed") {
            int value = 0;
            if (parsePositiveInt(args, value)) {
                state.speed = value;
                std::cout << "Marquee speed set to " << state.speed << " ms.\n";
            } else {
                std::cout << "Error: set_speed requires a positive integer "
                             "(milliseconds).\n";
            }
        } else if (command == "exit") {
            std::cout << "Terminating console...\n";
            return 0;
        } else {
            std::cout << "Unknown command: " << command << '\n';
        }

        std::cout << '\n';
    }
}
