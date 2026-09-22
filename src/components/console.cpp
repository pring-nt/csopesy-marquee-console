/**
 * @file console.cpp
 * @brief Implementation of the interactive command shell.
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

#include "console.h"

#include <sstream>
#include <utility>

#include "ascii_art.h"
#include "config.h"
#include "text_utils.h"

namespace marquee {

Console::Console(Font font, std::istream& in, std::ostream& out)
    : font_(std::move(font)), in_(in), out_(out) {}

void Console::printHeader() const {
    printPreview(kDefaultMarqueeText);
    out_ << '\n';
    out_ << "Group developer:\n";
    for (const char* developer : kDevelopers) {
        out_ << developer << '\n';
    }
    out_ << "\nVersion date: " << kVersionDate << "\n\n";
}

void Console::printHelp() const {
    out_ << "help            - displays the commands and its description\n"
         << "start_marquee   - starts the marquee \"animation\"\n"
         << "stop_marquee    - stops the marquee \"animation\"\n"
         << "set_text        - accepts a text input and displays it as a marquee\n"
         << "set_speed       - sets the marquee animation refresh in milliseconds\n"
         << "exit            - terminates the console\n";
}

void Console::printPreview(const std::string& text) const {
    printAsciiArt(text, font_, out_);
}

void Console::handleSetText(const std::string& args) {
    if (args.empty()) {
        out_ << "Error: set_text requires a non-empty string argument.\n";
        return;
    }
    marquee_.setText(args);
    out_ << "Text saved for marquee: " << marquee_.text() << '\n';
    printPreview(marquee_.text());
}

void Console::handleSetSpeed(const std::string& args) {
    int value = 0;
    if (parsePositiveInt(args, value)) {
        marquee_.setSpeed(value);
        out_ << "Marquee speed set to " << marquee_.speed() << " ms.\n";
    } else {
        out_ << "Error: set_speed requires a positive integer "
                "(milliseconds).\n";
    }
}

int Console::run() {
    printHeader();

    std::string input;
    while (true) {
        out_ << "Command> " << std::flush;
        if (!std::getline(in_, input)) {
            out_ << "\nTerminating console...\n";
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
            handleSetText(args);
        } else if (command == "start_marquee") {
            marquee_.start();
            out_ << "Marquee started. (Animation stub - not yet implemented)\n";
        } else if (command == "stop_marquee") {
            marquee_.stop();
            out_ << "Marquee stopped. (Animation stub - not yet implemented)\n";
        } else if (command == "set_speed") {
            handleSetSpeed(args);
        } else if (command == "exit") {
            out_ << "Terminating console...\n";
            return 0;
        } else {
            out_ << "Unknown command: " << command << '\n';
        }

        out_ << '\n';
    }
}

}  // namespace marquee
