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
 * driven in tests, and all marquee state - including the animation - is
 * delegated to @ref marquee::Marquee.
 *
 * The console owns the screen below the band. After every command it repaints
 * that area from scratch - header block, the output of the command that was
 * just run, and then the prompt - instead of letting lines pile up and scroll.
 * Nothing can therefore be overwritten by the animation, and the layout is
 * always the same. See Section 9.4 of SPECIFICATIONS.md.
 */

#ifndef CSOPESY_MARQUEE_CONSOLE_H
#define CSOPESY_MARQUEE_CONSOLE_H

#include <iostream>
#include <streambuf>
#include <string>

#include "font.h"
#include "marquee.h"
#include "terminal.h"

namespace marquee {

/// Reads commands from an input stream and writes replies to an output stream.
class Console {
public:
    /**
     * @brief Builds a console around a loaded font.
     *
     * The console puts its own locking buffer on @p out so the replies and
     * the animation frames cannot interleave, and takes the buffer back when
     * it is destroyed.
     *
     * @param font Font used for the header and @c set_text previews.
     * @param in Stream commands are read from; defaults to @c std::cin.
     * @param out Stream replies are written to; defaults to @c std::cout.
     */
    explicit Console(Font font,
                     std::istream& in = std::cin,
                     std::ostream& out = std::cout);

    /// Stops the marquee and gives @c out_ its original buffer back.
    ~Console();

    Console(const Console&) = delete;
    Console& operator=(const Console&) = delete;

    /**
     * @brief Paints the screen and runs the command loop until exit or EOF.
     *
     * Stops the marquee before returning, so no frame is drawn after the
     * console has said goodbye.
     *
     * @return Always zero.
     */
    int run();

private:
    /// Paints the band, the header block, the last reply, and the prompt.
    void paintScreen();

    /// Draws the band: the scrolling frame, or the text as plain art.
    void paintBand() const;

    /// Paints the pinned block under the band: the group names and version.
    void printHeader() const;

    /// Paints the command table shown by @c help into the reply.
    void printHelp();

    /// Handles @c set_text: validates, stores, and previews.
    void handleSetText(const std::string& args);

    /// Handles @c set_speed: validates and stores the interval.
    void handleSetSpeed(const std::string& args);

    /// Handles @c start_marquee: validates, confirms, and starts the thread.
    void handleStartMarquee();

    /// Handles @c stop_marquee: stops the thread.
    void handleStopMarquee();

    /// Stops the marquee, if one is running.
    void shutdownMarquee();

    Font font_;            ///< Font used for all art output.
    Marquee marquee_;      ///< Marquee state mutated by the commands.
    std::istream& in_;     ///< Input stream for the command loop.
    std::ostream& out_;    ///< Output stream for all replies.
    std::streambuf* originalOut_;  ///< Buffer @c out_ had on construction.
    SyncBuf sync_;         ///< Write lock shared with the animation thread.
    std::string screen_;   ///< Output of the last command, shown on screen.
};

}  // namespace marquee

#endif  // CSOPESY_MARQUEE_CONSOLE_H
