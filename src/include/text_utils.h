/**
 * @file text_utils.h
 * @brief Small, dependency-free string helpers used across the console.
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

#ifndef CSOPESY_MARQUEE_TEXT_UTILS_H
#define CSOPESY_MARQUEE_TEXT_UTILS_H

#include <string>

namespace marquee {

/**
 * @brief Removes leading and trailing whitespace.
 * @param text String to trim.
 * @return A copy of @p text without surrounding spaces, tabs, or newlines.
 */
std::string trim(const std::string& text);

/**
 * @brief Converts every character of @p text to uppercase.
 * @param text String to convert.
 * @return The uppercased copy, leaving non-letters unchanged.
 */
std::string toUpper(const std::string& text);

/**
 * @brief Removes trailing spaces from @p text.
 * @param text String to trim.
 * @return The string with any trailing spaces removed.
 */
std::string rightTrim(const std::string& text);

/**
 * @brief Parses a strictly positive integer.
 *
 * The whole string must be one number: leading and trailing whitespace is
 * ignored, but any other trailing character makes the parse fail.
 *
 * @param text String to parse.
 * @param[out] out Receives the parsed value on success.
 * @return @c true when @p text holds a positive integer, @c false otherwise.
 */
bool parsePositiveInt(const std::string& text, int& out);

}  // namespace marquee

#endif  // CSOPESY_MARQUEE_TEXT_UTILS_H
