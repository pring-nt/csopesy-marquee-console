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
 * The renderer is intentionally split from the printing: @ref
 * marquee::renderAsciiArt returns the finished rows as data, which is the
 * seam the future scrolling marquee needs (it will window and shift those
 * rows over time), while @ref marquee::printAsciiArt is the one-shot
 * convenience used by the console preview and the welcome header.
 */

#ifndef CSOPESY_MARQUEE_ASCII_ART_H
#define CSOPESY_MARQUEE_ASCII_ART_H

#include <iosfwd>
#include <string>
#include <vector>

#include "font.h"

namespace marquee {

/**
 * @brief Returns the visible console width in columns.
 *
 * Long ASCII-art rows wider than the console would be hard-wrapped by the
 * terminal mid-row, spilling glyph fragments onto the next visual line and
 * corrupting the block alignment. The width is used to cut off overflowing
 * art rows (the scrolling marquee reveals the hidden tail). The @c COLUMNS
 * environment variable wins when valid (useful for tests/redirection), then
 * the live console size is queried, otherwise 80 columns is assumed.
 *
 * @return Console width in columns, always positive.
 */
int getDisplayWidth();

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
 * @param maxWidth Maximum width of a returned row, in columns.
 * @return One entry per font row, or an empty vector for an unusable font.
 */
std::vector<std::string> renderAsciiArt(const std::string& text,
                                        const Font& font,
                                        int maxWidth);

/**
 * @brief Renders @p text and writes the rows to @p out.
 * @param text Text to render.
 * @param font Font used for the glyphs.
 * @param out Stream that receives one line per font row.
 */
void printAsciiArt(const std::string& text, const Font& font, std::ostream& out);

}  // namespace marquee

#endif  // CSOPESY_MARQUEE_ASCII_ART_H
