/**
 * @file asset_paths.cpp
 * @brief Implementation of the font-asset path resolver.
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

#include "asset_paths.h"

#include <fstream>

#include "config.h"

namespace {

/**
 * @brief Tests whether a file can be opened for reading.
 * @param path Candidate path.
 * @return @c true when the file exists and is readable.
 */
bool fileExists(const std::string& path) {
    const std::ifstream file(path.c_str());
    return static_cast<bool>(file);
}

}  // namespace

namespace marquee {

std::string resolveAssetPath(const std::string& fileName) {
    // Deliberately not const: a const local cannot be implicitly moved on
    // return, which would force a copy here.
    std::string nested = std::string(kAssetDir) + "/" + fileName;
    if (fileExists(nested)) {
        return nested;
    }
    return fileName;
}

}  // namespace marquee
