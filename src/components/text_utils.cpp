/**
 * @file text_utils.cpp
 * @brief Implementation of the shared string helpers.
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

#include "text_utils.h"

#include <cctype>
#include <istream>
#include <limits>
#include <sstream>

namespace marquee {

std::string trim(const std::string& text) {
    const char* kWhitespace = " \t\r\n";
    const std::string::size_type first = text.find_first_not_of(kWhitespace);
    if (first == std::string::npos) {
        return "";
    }
    const std::string::size_type last = text.find_last_not_of(kWhitespace);
    return text.substr(first, last - first + 1);
}

std::string toUpper(const std::string& text) {
    std::string result = text;
    for (char& ch : result) {
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
    return result;
}

std::string rightTrim(const std::string& text) {
    const std::string::size_type last = text.find_last_not_of(' ');
    if (last == std::string::npos) {
        return "";
    }
    return text.substr(0, last + 1);
}

bool parsePositiveInt(const std::string& text, int& out) {
    if (text.empty()) {
        return false;
    }
    std::istringstream stream(text);
    long long value = 0;
    stream >> value;
    if (stream.fail()) {
        return false;
    }
    stream >> std::ws;
    if (!stream.eof()) {
        return false;  // trailing characters are not allowed
    }
    if (value <= 0 || value > std::numeric_limits<int>::max()) {
        return false;
    }
    out = static_cast<int>(value);
    return true;
}

}  // namespace marquee
