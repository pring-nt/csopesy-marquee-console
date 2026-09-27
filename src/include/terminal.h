/**
 * @file terminal.h
 * @brief Cursor control, output locking, and timed waiting.
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
 * The animation runs on its own thread while the user keeps typing at the
 * prompt, so it needs three things the standard library does not offer: a way
 * to ask whether the output is an interactive console at all, escape sequences
 * that move the cursor and erase a line, and a way to keep a frame from being
 * spliced into the middle of a line the console is printing. That last one is
 * @ref marquee::SyncBuf. Keeping all of it here leaves
 * @ref marquee::Marquee and @ref marquee::Console free of platform code -
 * every @c ifdef lives in @c terminal.cpp.
 */

#ifndef CSOPESY_MARQUEE_TERMINAL_H
#define CSOPESY_MARQUEE_TERMINAL_H

#include <mutex>
#include <ostream>
#include <streambuf>
#include <string>
#include <vector>

namespace marquee {

/**
 * @brief Streambuf that serializes writes from several threads.
 *
 * The animation thread emits cursor control sequences, and the console thread
 * emits whole lines of text. Without a lock, a frame can land between two
 * writes of one line and the escape sequences end up in the middle of the
 * text - which is why the band would flicker and tear instead of scrolling.
 *
 * The buffer only guards writes; a stream installed on top of it must not be
 * read. It wraps the buffer the stream already had, so the console installs
 * one of these on its output stream: the lock is then shared by the console
 * and the animation thread, and the owner puts the original buffer back when
 * it goes away.
 */
class SyncBuf : public std::streambuf {
public:
    /// @param target Buffer that receives the characters written through it.
    explicit SyncBuf(std::streambuf* target);

    SyncBuf(const SyncBuf&) = delete;
    SyncBuf& operator=(const SyncBuf&) = delete;

protected:
    std::streamsize xsputn(const char* text, std::streamsize count) override;
    int_type overflow(int_type ch) override;
    int sync() override;

private:
    std::streambuf* target_;
    std::mutex mutex_;
};

/**
 * @brief Reports whether the in-place animation can be drawn.
 *
 * Drawing requires an interactive terminal: when the output is redirected to
 * a file or a pipe, there is no cursor to move and the escape sequences would
 * only pollute the captured text. On Windows the function also switches on
 * virtual-terminal processing for the output handle, without which @c cmd
 * would print the escape sequences literally.
 *
 * @return @c true when frames can be drawn in place.
 */
bool canDrawInPlace();

/**
 * @brief Returns the width the terminal window really shows.
 *
 * This is the *live* window width and nothing else: the frames are drawn at
 * absolute coordinates, so a width that does not match the window (a stale
 * @c COLUMNS, for instance) is not merely cosmetic - it makes the Windows
 * console scroll its window sideways, after which every following frame is
 * drawn at a shifted position.
 *
 * @return Window width in columns, or a fallback when it cannot be queried.
 */
int liveConsoleWidth();

/**
 * @brief Clears everything below the band.
 *
 * The console owns the whole screen and repaints it, so the only thing it has
 * to be careful about is the band: these rows are the animation's, and the
 * cursor is left on the first cleared row, ready for the repaint.
 *
 * @param bandRows Number of rows the band occupies at the top.
 * @param out Stream that receives the escape sequences, if any.
 */
void clearBelowBand(int bandRows, std::ostream& out);

/**
 * @brief Redraws the band in the title rows at the top of the screen.
 *
 * The band is the welcome title, not a new block further down: the cursor
 * position is saved, the screen is addressed at @c kMarqueeBandRow, every row
 * is erased and rewritten, and the cursor is put back where it was, so the
 * prompt and anything the user types stay exactly where they are. The whole
 * frame is assembled first and written in a single call, which keeps it from
 * being torn in half by another thread.
 *
 * @param rows Frame rows, one per band line.
 * @param out Stream that receives the escape sequences.
 */
void drawBand(const std::vector<std::string>& rows, std::ostream& out);

/**
 * @brief Waits for the given time without spinning the processor.
 * @param milliseconds Time to wait; values below one return immediately.
 */
void sleepMilliseconds(int milliseconds);

}  // namespace marquee

#endif  // CSOPESY_MARQUEE_TERMINAL_H
