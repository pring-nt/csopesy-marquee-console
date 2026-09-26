/**
 * @file ascii_art.cpp
 * @brief Implementation of the ASCII-art renderer.
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

#include "ascii_art.h"

#include <cstdlib>
#include <istream>
#include <ostream>
#include <sstream>

#include "config.h"
#include "text_utils.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace marquee {

namespace {

/**
 * @brief Joins the glyphs of @p text into one row per font row.
 *
 * The rows are returned uncut and untrimmed: the preview trims them itself,
 * and the animation needs the full band so it can take a window out of it.
 *
 * @param text Text to compose; it is uppercased first.
 * @param font Font used for the glyphs.
 * @return One entry per font row, or an empty vector for an unusable font.
 */
std::vector<std::string> composeBand(const std::string& text, const Font& font) {
    std::vector<std::string> rows;
    if (!font.valid() || text.empty()) {
        return rows;
    }
    const std::string upper = toUpper(text);
    const std::string gap(kGlyphGap, ' ');
    rows.reserve(font.height);
    for (int row = 0; row < font.height; ++row) {
        std::string composite;
        for (const char ch : upper) {
            composite += lookupGlyph(font, ch)[row];
            composite += gap;
        }
        rows.push_back(composite);
    }
    return rows;
}

/**
 * @brief Returns the width of @p text rendered as one uncut band.
 * @param text Text to render.
 * @param font Font used for the glyphs.
 * @return Band width in columns, or zero when the text or the font is empty.
 */
int measureBandWidth(const std::string& text, const Font& font) {
    const std::vector<std::string> rows = composeBand(text, font);
    if (rows.empty()) {
        return 0;
    }
    return static_cast<int>(rows[0].size());
}

}  // namespace

std::vector<std::string> renderAsciiArt(const std::string& text,
                                        const Font& font,
                                        int maxWidth) {
    std::vector<std::string> rows = composeBand(text, font);
    for (std::string& row : rows) {
        if (maxWidth >= 0 && static_cast<int>(row.size()) > maxWidth) {
            row.resize(static_cast<std::string::size_type>(maxWidth));
        }
        row = rightTrim(row);
    }
    return rows;
}

int measureBandPeriod(const std::string& text, const Font& font, int width) {
    const int bandWidth = measureBandWidth(text, font);
    if (bandWidth <= 0) {
        return 0;
    }
    // The band repeats every period columns, and the period decides whether
    // the text can be seen twice at once.
    //
    // Longer than the console: the period is the text itself, so the tail is
    // followed immediately by the head and the text reads as one continuous
    // stream. Shorter than the console: the period is the text plus a whole
    // console width, so the copy leaving on the right is gone before the copy
    // entering on the left shows up - never both, and never side by side.
    return bandWidth >= width ? bandWidth : bandWidth + width;
}

std::vector<std::string> renderScrollingFrame(const std::string& text,
                                               const Font& font,
                                               int width,
                                               int start) {
    const std::vector<std::string> cell = composeBand(text, font);
    std::vector<std::string> frame;
    const int period = measureBandPeriod(text, font, width);
    if (cell.empty() || width <= 0 || period <= 0) {
        return frame;
    }
    // The band is the padded cell twice over, so a window that starts anywhere
    // in the first period always has a full width to show.
    int wrapped = start % period;
    if (wrapped < 0) {
        wrapped += period;
    }
    const std::string::size_type from = static_cast<std::string::size_type>(wrapped);
    const std::string::size_type window = static_cast<std::string::size_type>(width);
    frame.reserve(cell.size());
    for (const std::string& row : cell) {
        std::string band = row;
        band.resize(static_cast<std::string::size_type>(period), ' ');
        band += band;
        frame.push_back(rightTrim(band.substr(from, window)));
    }
    return frame;
}

}  // namespace marquee
