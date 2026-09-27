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
 *
 * The loop does not print as it goes. A command fills @c screen_ with its
 * reply, and the screen is then painted from scratch: the band at the top, the
 * header block under it, the reply, and the prompt. That keeps the layout
 * identical no matter how many commands have been run, and keeps the console
 * out of the animation's rows.
 */

#include "console.h"

#include <sstream>
#include <utility>

#include "ascii_art.h"
#include "config.h"
#include "terminal.h"
#include "text_utils.h"

namespace marquee {

Console::Console(Font font, std::istream& in, std::ostream& out)
    : font_(std::move(font)),
      marquee_(font_, out),
      in_(in),
      out_(out),
      originalOut_(out.rdbuf()),
      sync_(originalOut_) {
    out_.rdbuf(&sync_);
}

Console::~Console() {
    // The marquee joins its thread on the way out; the original buffer is put
    // back so anything still flushing std::cout at exit is not left talking to
    // a buffer that no longer exists.
    marquee_.stop();
    out_.flush();
    out_.rdbuf(originalOut_);
}

void Console::paintBand() const {
    // While the animation runs it owns these rows, so they are left alone.
    if (marquee_.running()) {
        return;
    }
    drawBand(renderAsciiArt(marquee_.text(), font_, liveConsoleWidth()), out_);
}

void Console::printHeader() const {
    out_ << "Group developer:\n";
    for (const char* developer : kDevelopers) {
        out_ << developer << '\n';
    }
    out_ << "\nVersion date: " << kVersionDate << "\n\n";
}

void Console::printHelp() {
    screen_ =
        "help            - displays the commands and its description\n"
        "start_marquee   - starts the marquee \"animation\"\n"
        "stop_marquee    - stops the marquee \"animation\"\n"
        "set_text        - accepts a text input and displays it as a marquee\n"
        "set_speed       - sets the marquee animation refresh in milliseconds\n"
        "exit            - terminates the console\n";
}

void Console::paintScreen() {
    // The band's rows are the animation's; everything below them is repainted.
    clearBelowBand(font_.height, out_);
    printHeader();
    out_ << screen_ << std::flush;
    paintBand();
    out_ << "Command> " << std::flush;
}

void Console::handleSetText(const std::string& args) {
    if (args.empty()) {
        screen_ = "Error: set_text requires a non-empty string argument.\n";
        return;
    }
    marquee_.setText(args);
    // No art here: the band at the top of the screen is where the text is
    // shown, and it repaints with the new one on the next frame.
    screen_ = "Text saved for marquee: " + marquee_.text() + "\n";
}

void Console::handleSetSpeed(const std::string& args) {
    int value = 0;
    if (parsePositiveInt(args, value)) {
        marquee_.setSpeed(value);
        screen_ = "Marquee speed set to " + std::to_string(marquee_.speed()) +
                  " ms.\n";
    } else {
        screen_ = "Error: set_speed requires a positive integer (milliseconds).\n";
    }
}

void Console::handleStartMarquee() {
    if (marquee_.running()) {
        screen_ = "Marquee is already running.\n";
        return;
    }
    if (!font_.valid()) {
        screen_ =
            "Marquee not started. The ASCII-art font did not load; there is "
            "nothing to animate.\n";
        return;
    }
    if (!canDrawInPlace()) {
        screen_ =
            "Marquee not started. The animation needs an interactive "
            "terminal.\n";
        return;
    }
    marquee_.start();
    screen_ = "Marquee started. Use stop_marquee to stop it.\n";
}

void Console::handleStopMarquee() {
    shutdownMarquee();
    screen_ = "Marquee stopped.\n";
}

void Console::shutdownMarquee() {
    marquee_.stop();
}

int Console::run() {
    std::string input;
    while (true) {
        paintScreen();
        if (!std::getline(in_, input)) {
            shutdownMarquee();
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

        screen_.clear();
        if (command == "help") {
            printHelp();
        } else if (command == "set_text") {
            handleSetText(args);
        } else if (command == "start_marquee") {
            handleStartMarquee();
        } else if (command == "stop_marquee") {
            handleStopMarquee();
        } else if (command == "set_speed") {
            handleSetSpeed(args);
        } else if (command == "exit") {
            shutdownMarquee();
            out_ << "\nTerminating console...\n";
            return 0;
        } else {
            screen_ = "Unknown command: " + command + "\n";
        }
    }
}

}  // namespace marquee
