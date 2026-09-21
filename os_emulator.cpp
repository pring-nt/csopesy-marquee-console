/**
 * @file os_emulator.cpp
 * @brief Interactive console shell for the CSOPESY marquee.
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
 * conversion: the header and @c set_text output display plain text strings
 * directly. No font files are required.
 *
 * Build:
 *     g++ -std=c++17 os_emulator.cpp -o os_emulator
 */

#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

/// Text shown in the header and used until @c set_text replaces it.
const std::string kDefaultMarqueeText = "Welcome to CSOPESY!";

/// Refresh interval, in milliseconds, used until @c set_speed replaces it.
constexpr int kDefaultMarqueeSpeed = 200;

/// Date printed in the welcome header.
const std::string kVersionDate = "2026-09-18";

/// Developer names printed in the welcome header.
const std::vector<std::string> kDevelopers = {
    "Trinidad, Nathan",
    "Singh, Nathaniel",
    "Quilantang, Jann Miro",
    "Saguin, VL Kirsten Camille",
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
 * @brief Prints the welcome header: the title and the metadata block.
 */
void printHeader() {
    std::cout << kDefaultMarqueeText << '\n';
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
 * @brief Prints the header and runs the command loop.
 * @return Zero on a normal exit.
 */
int main() {
    printHeader();

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
            }
        } else if (command == "start_marquee") {
            state.running = true;
            std::cout
                << "Marquee started.\n";
        } else if (command == "stop_marquee") {
            state.running = false;
            std::cout
                << "Marquee stopped.\n";
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
