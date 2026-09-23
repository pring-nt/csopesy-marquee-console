/**
 * @file font.cpp
 * @brief Implementation of the ASCII-art font loader.
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

#include "font.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include "config.h"
#include "text_utils.h"

namespace {

/**
 * @brief Reads a text file into a vector of lines.
 *
 * Carriage returns are stripped so files with Windows line endings load the
 * same as files with Unix line endings. A file that cannot be opened produces
 * a warning on standard error.
 *
 * @param path File to read.
 * @return One entry per line, or an empty vector if the file cannot be opened.
 */
std::vector<std::string> readLines(const std::string& path) {
    std::vector<std::string> lines;
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Warning: could not open " << path
                  << " - falling back to blank glyphs.\n";
        return lines;
    }
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    return lines;
}

}  // namespace

namespace marquee {

Font loadFont(const std::string& fontPath, const std::string& orderPath) {
    Font font;

    const std::vector<std::string> orderLines = readLines(orderPath);
    const std::vector<std::string> artLines = readLines(fontPath);

    std::vector<char> order;
    order.reserve(orderLines.size());
    for (const std::string& line : orderLines) {
        order.push_back(line.empty() ? ' ' : line.front());
    }

    if (order.empty() || artLines.empty()) {
        return {};
    }

    const std::vector<std::string>::size_type height = artLines.size() / order.size();
    if (height == 0) {
        std::cerr << "Warning: " << fontPath
                  << " does not contain one glyph per character listed in "
                  << orderPath << ".\n";
        return {};
    }
    if (artLines.size() % order.size() != 0) {
        std::cerr << "Warning: " << fontPath << " has " << artLines.size()
                  << " lines for " << order.size()
                  << " characters - expected a multiple of " << order.size()
                  << " (" << order.size() * height << " lines); ignoring "
                  << (artLines.size() % order.size())
                  << " trailing line(s).\n";
    }
    font.height = static_cast<int>(height);

    for (const std::string& line : artLines) {
        font.width = std::max(font.width, static_cast<int>(rightTrim(line).size()));
    }
    font.blank = Glyph(font.height, std::string(kBlankWidth, ' '));

    for (std::vector<char>::size_type i = 0; i < order.size(); ++i) {
        std::string::size_type natural = 0;
        for (std::string::size_type row = 0; row < height; ++row) {
            natural = std::max(
                natural, rightTrim(artLines[i * height + row]).size());
        }
        if (natural == 0) {
            // All-blank block (e.g. the explicit space glyph): keep the
            // standard blank width instead of collapsing to one column.
            natural = static_cast<std::string::size_type>(kBlankWidth);
        }
        Glyph glyph;
        glyph.reserve(height);
        for (std::string::size_type row = 0; row < height; ++row) {
            std::string text = rightTrim(artLines[i * height + row]);
            text.resize(natural, ' ');
            glyph.push_back(text);
        }
        font.glyphs[order[i]] = glyph;
    }
    return font;
}

const Glyph& lookupGlyph(const Font& font, char ch) {
    const auto it = font.glyphs.find(ch);
    if (it != font.glyphs.end() && !it->second.empty()) {
        return it->second;
    }
    return font.blank;
}

}  // namespace marquee
