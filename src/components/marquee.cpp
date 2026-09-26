/**
 * @file marquee.cpp
 * @brief Implementation of the marquee state, lifecycle, and animation.
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
 * The animation loop lives here, next to the text, speed, and running flag it
 * reads. Each iteration takes a snapshot of the state under the lock, asks the
 * renderer for one window over the band, hands it to the terminal module, and
 * waits one refresh interval. The snapshot is what lets @c set_text and
 * @c set_speed retarget a running marquee without a data race.
 */

#include "marquee.h"

#include <string>
#include <utility>

#include "ascii_art.h"
#include "terminal.h"

namespace {

/// Longest single sleep of the animation thread, in milliseconds. The wait
/// between frames is cut into chunks this long so that stop_marquee does not
/// have to wait out a long interval before the thread notices.
constexpr int kWakeUpGranularityMs = 20;

}  // namespace

namespace marquee {

Marquee::Marquee(const Font& font, std::ostream& out, std::string text, int speedMs)
    : font_(font),
      out_(out),
      // Not const: the string is returned by the console to build the
      // "Text saved for marquee:" line, and const would block the move.
      text_(std::move(text)),
      speedMs_(speedMs),
      running_(false) {}

Marquee::~Marquee() {
    stop();
}

const std::string& Marquee::text() const {
    return text_;
}

int Marquee::speed() const {
    return speedMs_;
}

bool Marquee::running() const {
    return running_;
}

void Marquee::setText(const std::string& text) {
    const std::lock_guard<std::mutex> lock(stateMutex_);
    text_ = text;
}

void Marquee::setSpeed(int speedMs) {
    const std::lock_guard<std::mutex> lock(stateMutex_);
    speedMs_ = speedMs;
}

void Marquee::start() {
    {
        const std::lock_guard<std::mutex> lock(stateMutex_);
        if (running_) {
            return;
        }
        running_ = true;
    }
    worker_ = std::thread(&Marquee::animate, this);
}

void Marquee::stop() {
    {
        const std::lock_guard<std::mutex> lock(stateMutex_);
        running_ = false;
    }
    if (worker_.joinable()) {
        worker_.join();
    }
}

bool Marquee::isRunning() const {
    const std::lock_guard<std::mutex> lock(stateMutex_);
    return running_;
}

void Marquee::waitForNextFrame(int milliseconds) const {
    int remaining = milliseconds;
    while (remaining > 0 && isRunning()) {
        const int chunk =
            remaining > kWakeUpGranularityMs ? kWakeUpGranularityMs : remaining;
        sleepMilliseconds(chunk);
        remaining -= chunk;
    }
}

void Marquee::animate() {
    if (!font_.valid()) {
        // Nothing to draw. Clear the flag directly: calling stop() here would
        // try to join the thread that is running this very function.
        const std::lock_guard<std::mutex> lock(stateMutex_);
        running_ = false;
        return;
    }

    // How far the band has travelled, in columns. The band repeats every
    // period columns, so the travel wraps at the period and the wrap is
    // invisible. Only the thread touches it, so the console never has to
    // synchronize the animation's progress.
    int travel = 0;
    while (isRunning()) {
        std::string text;
        int speedMs = 0;
        {
            const std::lock_guard<std::mutex> lock(stateMutex_);
            text = text_;
            speedMs = speedMs_;
        }

        // The console width is read every frame, so resizing the window
        // re-cuts the band immediately. The live width is used rather than
        // liveConsoleWidth(): the frames are drawn at absolute coordinates, and
        // a width that does not match the window (a stale COLUMNS, say) makes
        // the Windows console scroll sideways instead of merely being too wide.
        const int width = liveConsoleWidth();
        const int period = measureBandPeriod(text, font_, width);
        if (period > 0) {
            // The window walks backwards through the band, which is what makes
            // the text travel to the right.
            const int start = (period - travel) % period;
            drawBand(renderScrollingFrame(text, font_, width, start), out_);
            travel = (travel + 1) % period;
        }
        waitForNextFrame(speedMs);
    }
}

}  // namespace marquee
