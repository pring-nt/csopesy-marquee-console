/**
 * @file terminal.cpp
 * @brief Implementation of the cursor control, output locking, and waiting.
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
 * The only non-portable part of the whole project. Everything is either a
 * plain ASCII escape sequence - so a redirected capture stays readable - or a
 * single platform call guarded by @c ifdef. The file also owns the process-wide
 * state of the screen, because there is only one screen: whether the band is
 * pinned, and the buffer geometry to put back when it is released.
 */

#include "terminal.h"

#include <cstdio>
#include <ostream>
#include <string>
#include <vector>
#include "config.h"
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace {

// Escape sequences used to draw the band and to pin it. Character arrays, not
// std::string, so they are constant-initialized and cost nothing at startup.
// All of these sequences start with the control sequence introducer; the rest
// of each one is appended as numbers and a final letter.
constexpr char kCsi[] = "\033[";
constexpr char kSaveCursor[] = "\033[s";
constexpr char kRestoreCursor[] = "\033[u";
constexpr char kEraseLine[] = "\033[K";
constexpr char kCarriageReturn[] = "\r";
constexpr char kEraseToEnd[] = "\033[J";

/// Screen width assumed when the terminal will not report one.
constexpr int kFallbackColumns = 80;

#ifdef _WIN32
// Older SDK headers predate the constant; the value has not changed.
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
constexpr DWORD kEnableVirtualTerminalProcessing = 0x0004;
#else
constexpr DWORD kEnableVirtualTerminalProcessing = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
#endif
#endif

/**
 * @brief Top-left cell of the band, 1-based, as the terminal counts rows.
 *
 * POSIX terminals address the *visible* screen, so the band starts at the
 * first row on screen. The Windows console host addresses the whole buffer
 * instead, where row 1 is the top of the scrollback, so the window rectangle
 * has to be asked for: addressing the buffer top would drag the view back to
 * the oldest output on every frame.
 *
 * @param[out] x Column of the first band cell.
 * @param[out] y Row of the first band cell.
 */
void bandOrigin(int& x, int& y) {
    x = 1;
    y = marquee::kMarqueeBandRow;
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info) != 0) {
        x = info.srWindow.Left + 1;
        y = info.srWindow.Top + 1;
    }
#endif
}

/**
 * @brief Tells whether standard output is a real console.
 * @return @c true when output is attached to a terminal.
 */
bool isInteractive() {
#ifdef _WIN32
    return _isatty(_fileno(stdout)) != 0;
#else
    return isatty(STDOUT_FILENO) != 0;
#endif
}

/**
 * @brief Returns the number of columns the terminal shows.
 * @return Screen width in columns, or a fallback when it cannot be queried.
 */
int consoleWidth() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info) != 0) {
        return info.srWindow.Right - info.srWindow.Left + 1;
    }
    return kFallbackColumns;
#else
    struct winsize size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0 && size.ws_col > 0) {
        return static_cast<int>(size.ws_col);
    }
    return kFallbackColumns;
#endif
}

}  // namespace

namespace marquee {

SyncBuf::SyncBuf(std::streambuf* target)
    : target_(target) {}

std::streambuf::int_type SyncBuf::overflow(int_type ch) {
    const std::lock_guard<std::mutex> lock(mutex_);
    return target_->sputc(static_cast<char>(ch));
}

std::streamsize SyncBuf::xsputn(const char* text, std::streamsize count) {
    const std::lock_guard<std::mutex> lock(mutex_);
    return target_->sputn(text, count);
}

int SyncBuf::sync() {
    const std::lock_guard<std::mutex> lock(mutex_);
    return target_->pubsync();
}

bool canDrawInPlace() {
    if (!isInteractive()) {
        return false;
    }
#ifdef _WIN32
    // cmd.exe interprets escape sequences only after virtual-terminal
    // processing is enabled on the output handle.
    const HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(handle, &mode) != 0) {
        if ((mode & kEnableVirtualTerminalProcessing) == 0) {
            SetConsoleMode(handle, mode | kEnableVirtualTerminalProcessing);
        }
    }
#endif
    return true;
}

int liveConsoleWidth() {
    return consoleWidth();
}

void clearBelowBand(int bandRows, std::ostream& out) {
    if (bandRows < 0) {
        return;
    }
#ifdef _WIN32
    (void)out;
    // The rows are filled through the console API rather than with an
    // erase-to-end-of-screen sequence, so the band is never touched even if the
    // screen was never written to.
    const HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(handle, &info) != 0) {
        for (int row = info.srWindow.Top + bandRows;
             row <= info.srWindow.Bottom; ++row) {
            COORD where;
            where.X = static_cast<SHORT>(info.srWindow.Left);
            where.Y = static_cast<SHORT>(row);
            DWORD written = 0;
            FillConsoleOutputCharacterA(handle, ' ',
                                        static_cast<DWORD>(info.dwSize.X),
                                        where, &written);
        }
        COORD top;
        top.X = static_cast<SHORT>(info.srWindow.Left);
        top.Y = static_cast<SHORT>(info.srWindow.Top + bandRows);
        SetConsoleCursorPosition(handle, top);
    }
#else
    std::string clear = kCsi;
    clear += std::to_string(marquee::kMarqueeBandRow + bandRows);
    clear += ";1H";
    clear += kEraseToEnd;
    out << clear << std::flush;
#endif
}

void drawBand(const std::vector<std::string>& rows, std::ostream& out) {
    if (rows.empty()) {
        return;
    }
    int x = 1;
    int y = 0;
    bandOrigin(x, y);
    // A row wider than the window makes the Windows console scroll its window
    // sideways, and every later frame would then be drawn at a shifted
    // position, so the rows are cut to what actually fits. At least one column
    // always fits, whatever the console reports.
    const int columnsLeft = consoleWidth() - (x - 1);
    const std::string::size_type limit =
        columnsLeft > 0 ? static_cast<std::string::size_type>(columnsLeft) : 1;
    // Built as one buffer so the whole frame reaches the terminal in a single
    // write: the console thread cannot print into the middle of it, and the
    // cursor never rests on a half-drawn band.
    std::string frame;
    frame += kSaveCursor;
    frame += kCsi;
    frame += std::to_string(y);
    frame += ';';
    frame += std::to_string(x);
    frame += 'H';
    for (std::string::size_type i = 0; i < rows.size(); ++i) {
        // Carriage return first: a row as wide as the window leaves the cursor
        // in the last column, where an erase-to-end-of-line would clear nothing.
        frame += kCarriageReturn;
        frame += kEraseLine;
        frame += rows[i].size() > limit ? rows[i].substr(0, limit) : rows[i];
        // No newline after the last row: it would push the whole screen up by
        // one line on every frame.
        if (i + 1 < rows.size()) {
            frame += '\n';
        }
    }
    frame += kRestoreCursor;
    out << frame << std::flush;
}

void sleepMilliseconds(int milliseconds) {
    if (milliseconds <= 0) {
        return;
    }
#ifdef _WIN32
    ::Sleep(static_cast<DWORD>(milliseconds));
#else
    // Chunked because usleep() is only defined up to one second.
    int remaining = milliseconds;
    while (remaining > 0) {
        const int chunk = remaining > 1000 ? 1000 : remaining;
        usleep(static_cast<useconds_t>(chunk) * 1000);
        remaining -= chunk;
    }
#endif
}

}  // namespace marquee
