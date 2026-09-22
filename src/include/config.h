/**
 * @file config.h
 * @brief Compile-time configuration shared by the marquee console.
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
 * Centralises the values that used to sit at the top of @c main.cpp: the font
 * asset names, the default marquee text and speed, the welcome-header
 * metadata, and the glyph layout constants. Every module can include this
 * header without pulling in any implementation.
 *
 * All values are constant-initialized @c constexpr objects. The string values
 * are plain @c char arrays rather than @c std::string / @c std::vector on
 * purpose: a namespace-scope @c std::string needs dynamic initialization
 * before @c main, and a throw there (e.g. @c std::bad_alloc) terminates the
 * program before any handler can catch it. Character arrays are initialized
 * at compile time and cost nothing at startup.
 */

#ifndef CSOPESY_MARQUEE_CONFIG_H
#define CSOPESY_MARQUEE_CONFIG_H

namespace marquee {

/// Directory that holds the font assets, relative to the working directory.
constexpr char kAssetDir[] = "assets";

/// Name of the file holding the raw glyph blocks, top to bottom.
constexpr char kFontFile[] = "ascii_art.txt";

/// Name of the file listing the glyph characters, one per line.
constexpr char kOrderFile[] = "characters.txt";

/// Text shown in the header and used until @c Marquee::setText replaces it.
constexpr char kDefaultMarqueeText[] = "CSOPESY";

/// Refresh interval, in milliseconds, used until @c Marquee::setSpeed
/// replaces it.
constexpr int kDefaultMarqueeSpeed = 200;

/// Date printed in the welcome header.
constexpr char kVersionDate[] = "2026-09-18";

/// Developer names printed in the welcome header.
constexpr const char* kDevelopers[] = {
    "Trinidad, Nathan",
    "Singh, Nathaniel",
    "Quilantang, Jann Miro",
    "Saguin, VL Kirsten Camille",
};

/// Columns of separation inserted between adjacent glyphs when rendering.
/// Glyphs are stored at their natural ink width (trailing padding stripped),
/// so this small fixed gap replaces the dead space the old global-width
/// padding produced around narrow glyphs such as 'I' or '!'.
constexpr int kGlyphGap = 1;

/// Width of the blank block used for spaces and unknown characters.
constexpr int kBlankWidth = 4;

}  // namespace marquee

#endif  // CSOPESY_MARQUEE_CONFIG_H
