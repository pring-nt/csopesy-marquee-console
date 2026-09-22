/**
 * @file font.h
 * @brief ASCII-art font model and loader.
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
 * Owns the glyph data structures and the logic that turns the raw
 * @c ascii_art.txt / @c characters.txt pair into a renderable font. The
 * renderer and the marquee never parse files themselves; they only consume
 * the @ref marquee::Font produced here.
 */

#ifndef CSOPESY_MARQUEE_FONT_H
#define CSOPESY_MARQUEE_FONT_H

#include <map>
#include <string>
#include <vector>

namespace marquee {

/// One glyph: its rows, top row first.
using Glyph = std::vector<std::string>;

/// Maps each supported character to its glyph.
using FontMap = std::map<char, Glyph>;

/// A parsed font ready for rendering.
struct Font {
    int height = 0;   ///< Number of rows in every glyph.
    int width = 0;    ///< Ink width of the widest glyph.
    FontMap glyphs;   ///< Glyph looked up by character.
    Glyph blank;      ///< Blank block used for spaces and unknown characters.

    /// @return @c true when the font holds at least one usable glyph.
    bool valid() const { return height > 0 && width > 0; }
};

/**
 * @brief Builds the font from the glyph and order files.
 *
 * The glyph order file lists one character per line; a blank line stands for a
 * space. The art file stores the matching glyph blocks in the same order, so
 * the glyph height is the number of art lines divided by the number of
 * characters, and the glyph ink width is the widest ink row found. Each glyph
 * keeps only its natural ink width (trailing padding stripped) so narrow
 * glyphs are not stretched to the width of the widest one (e.g. 'W'); a
 * single-column gap inserted at render time keeps characters separated.
 *
 * A file that cannot be opened produces a warning on standard error and an
 * empty font.
 *
 * @param fontPath File containing the raw glyph blocks.
 * @param orderPath File listing one character per glyph.
 * @return The parsed font, or a font with no glyphs if either file is unusable.
 */
Font loadFont(const std::string& fontPath, const std::string& orderPath);

/**
 * @brief Looks up the glyph for a character.
 * @param font Font to search.
 * @param ch Character to render.
 * @return The character's glyph, or the blank block when it is not mapped.
 */
const Glyph& lookupGlyph(const Font& font, char ch);

}  // namespace marquee

#endif  // CSOPESY_MARQUEE_FONT_H
