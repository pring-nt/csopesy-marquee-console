/**
 * @file asset_paths.h
 * @brief Locates the font assets on disk.
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
 * The font data lives in @c assets/ in the repository, and CMake copies both
 * files next to the built executable. The console therefore accepts either
 * layout: it prefers @c assets/<name> and falls back to @c <name> in the
 * working directory, so a run from the repository root and a run from the
 * build directory both find the same font.
 */

#ifndef CSOPESY_MARQUEE_ASSET_PATHS_H
#define CSOPESY_MARQUEE_ASSET_PATHS_H

#include <string>

namespace marquee {

/**
 * @brief Resolves an asset file name to a readable path.
 *
 * Tries @c assets/<fileName> first, then @c <fileName> in the working
 * directory. When neither exists the plain name is returned so the caller's
 * error message points at the documented location.
 *
 * @param fileName Bare asset file name, e.g. @c "ascii_art.txt".
 * @return A path to use when opening the asset.
 */
std::string resolveAssetPath(const std::string& fileName);

}  // namespace marquee

#endif  // CSOPESY_MARQUEE_ASSET_PATHS_H
