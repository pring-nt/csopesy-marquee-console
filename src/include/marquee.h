/**
 * @file marquee.h
 * @brief Marquee state, lifecycle, and the scrolling animation.
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
 * the refresh interval, and the running flag - behind a small interface, and
 * runs the scrolling animation on its own thread. The console only asks
 * @ref marquee::Marquee to start and stop; it never flips a bool itself and
 * never knows how a frame is drawn.
 *
 * The animation is a worker thread so the user can keep working: the command
 * loop keeps reading, @c set_text and @c set_speed change the animation while
 * it runs, and @c stop_marquee ends it. The thread only ever writes through
 * the stream's buffer, and @ref marquee::drawBand puts every frame back where
 * the cursor was, so the two never fight over the screen.
 *
 * @c text(), @c speed(), and @c running() belong to the console thread, which
 * is the only thread that may call them while the animation is running; the
 * worker reads the same values through its own lock. @ref marquee::Marquee
 * joins the thread in @ref marquee::Marquee::stop and in its destructor, so
 * no frame is ever drawn after the console has moved on.
 */

#ifndef CSOPESY_MARQUEE_MARQUEE_H
#define CSOPESY_MARQUEE_MARQUEE_H

#include <mutex>
#include <iosfwd>
#include <string>
#include <thread>

#include "config.h"
#include "font.h"

namespace marquee {

/// Marquee settings, lifecycle, and the scrolling animation.
class Marquee {
public:
    /**
     * @brief Builds a marquee around a font and the stream it draws on.
     *
     * The font and the stream are borrowed for the lifetime of the marquee,
     * which is why the class is not copyable.
     *
     * @param font Font the animation renders its glyphs from.
     * @param out Stream the frames are written to; the console installs its
     *            locking buffer on it, so console and animation never
     *            interleave their output.
     * @param text Initial marquee text.
     * @param speedMs Initial refresh interval in milliseconds.
     */
    Marquee(const Font& font,
            std::ostream& out,
            std::string text = kDefaultMarqueeText,
            int speedMs = kDefaultMarqueeSpeed);

    Marquee(const Marquee&) = delete;
    Marquee& operator=(const Marquee&) = delete;

    /// Stops the animation, if one is running, before the marquee is dropped.
    ~Marquee();

    /// @return The text that scrolls when the marquee runs.
    const std::string& text() const;

    /// @return The refresh interval, in milliseconds.
    int speed() const;

    /// @return @c true while the marquee is active.
    bool running() const;

    /**
     * @brief Replaces the marquee text.
     *
     * Safe to call while the animation runs: the next frame already uses the
     * new text.
     *
     * @param text New text; expected non-empty (the console validates it).
     */
    void setText(const std::string& text);

    /**
     * @brief Replaces the refresh interval.
     *
     * Safe to call while the animation runs: the next wait uses the new
     * interval.
     *
     * @param speedMs New interval in milliseconds; expected positive.
     */
    void setSpeed(int speedMs);

    /// Marks the marquee active and starts the animation thread.
    void start();

    /// Marks the marquee inactive and waits for the thread to finish.
    void stop();

private:
    /// Body of the animation thread: one frame per refresh interval.
    void animate();

    /// @return @c true while the marquee is active, read under the lock.
    bool isRunning() const;

    /// Waits one interval, waking early if the marquee was stopped.
    void waitForNextFrame(int milliseconds) const;

    const Font& font_;       ///< Font the animation renders from.
    std::ostream& out_;      ///< Stream every frame is written to.
    std::string text_;       ///< Current marquee text.
    int speedMs_;            ///< Refresh interval in milliseconds.
    bool running_;           ///< Whether the marquee is active.
    std::thread worker_;     ///< Animation thread, joined by stop().
    mutable std::mutex stateMutex_;  ///< Guards the state the worker reads.
};

}  // namespace marquee

#endif  // CSOPESY_MARQUEE_MARQUEE_H
