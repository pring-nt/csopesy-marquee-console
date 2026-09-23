/**
 * @file marquee.cpp
 * @brief Implementation of the marquee state and lifecycle.
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
 * Phase 1 only records the requested state. The scrolling animation belongs
 * here: @ref marquee::Marquee::start will eventually spawn or drive the tick
 * loop, using @c speedMs_ as the refresh interval and the rows returned by
 * @ref marquee::renderAsciiArt as the frame it shifts.
 */

#include "marquee.h"

#include <utility>

#include "config.h"

namespace marquee {

Marquee::Marquee()
    : text_(kDefaultMarqueeText),
      speedMs_(kDefaultMarqueeSpeed),
      running_(false) {}

Marquee::Marquee(std::string text, int speedMs)
    : text_(std::move(text)), speedMs_(speedMs), running_(false) {}

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
    text_ = text;
}

void Marquee::setSpeed(int speedMs) {
    speedMs_ = speedMs;
}

void Marquee::start() {
    running_ = true;
}

void Marquee::stop() {
    running_ = false;
}

}  // namespace marquee
