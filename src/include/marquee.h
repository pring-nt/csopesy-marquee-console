/**
 * @file marquee.h
 * @brief In-memory marquee state and lifecycle.
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
 * Holds the three values the console keeps across commands - the marquee text,
 * the refresh interval, and the running flag - behind a small interface. This
 * is the seam the animation engine will grow into: the console already asks
 * @ref marquee::Marquee to start and stop rather than flipping a bool itself,
 * so the scrolling loop can be added in @c marquee.cpp without touching
 * command parsing.
 */

#ifndef CSOPESY_MARQUEE_MARQUEE_H
#define CSOPESY_MARQUEE_MARQUEE_H

#include <string>

namespace marquee {

/// Marquee settings and lifecycle carried across console commands.
class Marquee {
public:
    /// Starts with the default text, default speed, and the marquee stopped.
    Marquee();

    /**
     * @brief Starts with explicit text and speed, and the marquee stopped.
     * @param text Initial marquee text.
     * @param speedMs Initial refresh interval in milliseconds.
     */
    Marquee(std::string text, int speedMs);

    /// @return The text that will scroll when the marquee runs.
    const std::string& text() const;

    /// @return The refresh interval, in milliseconds.
    int speed() const;

    /// @return @c true while the marquee is active.
    bool running() const;

    /**
     * @brief Replaces the marquee text.
     * @param text New text; expected non-empty (the console validates it).
     */
    void setText(const std::string& text);

    /**
     * @brief Replaces the refresh interval.
     * @param speedMs New interval in milliseconds; expected positive.
     */
    void setSpeed(int speedMs);

    /// Marks the marquee active. Phase 1 records the request only.
    void start();

    /// Marks the marquee inactive. Phase 1 records the request only.
    void stop();

private:
    std::string text_;   ///< Current marquee text.
    int speedMs_;        ///< Refresh interval in milliseconds.
    bool running_;       ///< Whether the marquee is active.
};

}  // namespace marquee

#endif  // CSOPESY_MARQUEE_MARQUEE_H
