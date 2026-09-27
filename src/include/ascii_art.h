/**
 * @file ascii_art.h
 * @brief Turns marquee text into composite ASCII-art rows.
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
 * The renderer returns finished rows as data and nothing else: the console
 * draws them into the band, the animation shifts them one column per frame.
 *
 * Two renderings of the same text exist on purpose.
 * @ref marquee::renderAsciiArt cuts the rows at the console width and stops -
 * that is the whole text, for the band when the marquee is stopped.
 * @ref marquee::renderScrollingFrame returns a moving window over the uncut
 * band, which is what the animation scrolls.
 */

#ifndef CSOPESY_MARQUEE_ASCII_ART_H
#define CSOPESY_MARQUEE_ASCII_ART_H

#include <string>
#include <vector>

#include "font.h"

namespace marquee {

/**
 * @brief Renders @p text as one composite row per glyph row.
 *
 * The text is uppercased, each character is replaced by its natural-width
 * glyph, and the glyphs are concatenated row by row with a single-column gap.
 * Any composite row wider than @p maxWidth is cut off instead of wrapped: the
 * marquee scrolls, so the hidden tail is revealed by the animation rather than
 * spilling onto extra lines and corrupting the block layout. Trailing spaces
 * are removed from each row.
 *
 * @param text Text to render.
 * @param font Font used for the glyphs.
 * @param maxWidth Maximum width of a returned row, in columns; the caller
 *                 passes the live console width. A negative value means "no
 *                 cut", which is what the scrolling frames ask for.
 * @return One entry per font row, or an empty vector for an unusable font.
 */
std::vector<std::string> renderAsciiArt(const std::string& text,
                                        const Font& font,
                                        int maxWidth);

/**
 * @brief Returns the distance, in columns, after which the band repeats.
 *
 * The period is what keeps the text from appearing twice at the same time. A
 * text longer than the console repeats after its own width, so the head
 * follows the tail seamlessly. A text shorter than the console repeats after
 * its own width plus a full console, so the copy leaving on the right is gone
 * before the copy entering on the left arrives.
 *
 * @param text Text that scrolls.
 * @param font Font used for the glyphs.
 * @param width Console width in columns.
 * @return Repeat distance in columns, or zero for an unusable text or font.
 */
int measureBandPeriod(const std::string& text, const Font& font, int width);

/**
 * @brief Renders one frame of the scrolling band.
 *
 * The band is the rendered text padded to @c measureBandPeriod and repeated
 * twice, so a window that starts anywhere inside the first period is always
 * full and the repeat is seamless. The window is @p width columns wide and
 * starts @p start columns into the band; @p start is wrapped into one period,
 * and because the band repeats, the wrap is invisible. That is also how the
 * text travels to the right: the window walks backwards through the band, so
 * each character appears one column further right every frame.
 *
 * @param text Text that scrolls.
 * @param font Font used for the glyphs.
 * @param width Width of the window in columns; values below one give no rows.
 * @param start Column the window starts at; negatives are wrapped, not clamped.
 * @return One entry per font row, or an empty vector for an unusable font.
 */
std::vector<std::string> renderScrollingFrame(const std::string& text,
                                               const Font& font,
                                               int width,
                                               int start);

}  // namespace marquee

#endif  // CSOPESY_MARQUEE_ASCII_ART_H
