/**
 * @file console.h
 * @brief The interactive command shell.
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
 * Owns everything the user sees: the welcome header, the @c help table, the
 * @c Command> read-eval-print loop, and every confirmation/error string. All
 * console output goes through the injected @c std::ostream so the loop can be
 * driven in tests, and all marquee state is delegated to
 * @ref marquee::Marquee.
 */

#ifndef CSOPESY_MARQUEE_CONSOLE_H
#define CSOPESY_MARQUEE_CONSOLE_H

#include <iostream>
#include <string>

#include "font.h"
#include "marquee.h"

namespace marquee {

/// Reads commands from an input stream and writes replies to an output stream.
class Console {
public:
    /**
     * @brief Builds a console around a loaded font.
     * @param font Font used for the header and @c set_text previews.
     * @param in Stream commands are read from; defaults to @c std::cin.
     * @param out Stream replies are written to; defaults to @c std::cout.
     */
    explicit Console(Font font,
                     std::istream& in = std::cin,
                     std::ostream& out = std::cout);

    /**
     * @brief Prints the header and runs the command loop until exit or EOF.
     * @return Always zero.
     */
    int run();

private:
    /// Prints the title art and the group metadata block.
    void printHeader() const;

    /// Prints the command table shown by @c help.
    void printHelp() const;

    /// Handles @c set_text: validates, stores, confirms, and previews.
    void handleSetText(const std::string& args);

    /// Handles @c set_speed: validates and stores the interval.
    void handleSetSpeed(const std::string& args);

    /// Prints the ASCII-art preview of @p text.
    void printPreview(const std::string& text) const;

    Font font_;            ///< Font used for all art output.
    Marquee marquee_;      ///< Marquee state mutated by the commands.
    std::istream& in_;     ///< Input stream for the command loop.
    std::ostream& out_;    ///< Output stream for all replies.
};

}  // namespace marquee

#endif  // CSOPESY_MARQUEE_CONSOLE_H
