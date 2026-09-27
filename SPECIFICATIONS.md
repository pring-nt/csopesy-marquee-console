# CSOPESY - Console CLI Specifications

**Group Developers:** Trinidad, Nathan, Singh, Nathaniel, Quilantang, Jann Miro, Saguin, Kirsten Camille  
**Version Date:** 2026-09-18  
**Language:** C++ (Standard: C++17 or later)

---

## 1. Overview
CSOPESY is a console-based command-line interface (CLI) written in C++. This document specifies the behavior of the Phase 1 implementation: a minimal interactive shell that lays the groundwork for a future marquee animation system, together with the scrolling marquee itself.

The program must compile and run as a single executable with no external runtime dependencies beyond the C++ Standard Library.

The Phase 1 sources are organised as small modules - headers in `src/include/` with their implementations in `src/components/` (console shell, marquee state and animation, ASCII-art renderer, font loader, text helpers, asset resolver, terminal cursor control, and compile-time config) - so nothing grows into a single translation unit.

---

## 2. Program Entry & Welcome Header
Upon launch, the program shall print a styled welcome header before entering the command loop.

### 2.1 ASCII Art Title
The string `"CSOPESY"` must be rendered in ASCII art text using the font loaded from `ascii_art.txt` - no external libraries (e.g., `figlet`) are permitted. The ASCII art is constructed by mapping characters to the font lookup table loaded at startup. The glyph set and file format are described in Section 7.

> **Note:** These rows are the marquee band. `start_marquee` scrolls the stored
> text through exactly them, and `set_text` changes what the band shows at
> runtime. The metadata block of Section 2.2 stays pinned directly underneath,
> and the area below it is repainted after every command (Section 9.2).

### 2.2 Metadata Block
Immediately after the ASCII art, print the following plain-text block:

```text
Group developer:
Trinidad, Nathan
Singh, Nathaniel
Quilantang, Jann Miro
Saguin, Kirsten Camille

Version date: YYYY-MM-DD
```
Fields are populated at compile time (hardcoded strings are acceptable for Phase 1).

---

## 3. Command Loop
After the header, the program enters an infinite read-eval-print loop (REPL):

```text
Command> _
```

### 3.1 Prompt Behavior
1. Repaint the screen - band, header block, the previous reply - then print
   `Command> ` with a trailing space and no newline (Section 9.2).
2. Read an entire line of input using `std::getline(std::cin, input)`.
3. Parse the first token as the command name; everything after the first space is treated as arguments.
4. Commands are case-sensitive (all lowercase as shown).
5. Leading/trailing whitespace on the full input line should be trimmed before parsing.

### 3.2 Unrecognized Command
If the command token does not match any recognized command, print:

```text
Unknown command: <token>
```
Then return to the `Command> ` prompt. Do not terminate the program.

---

## 4. Command Reference

### 4.1 help
* **Syntax:** `help`
* **Arguments:** None
* **Description:** Prints a list of all available commands with a one-line description each.
* **Required output format:**
```text
help            - displays the commands and its description
start_marquee   - starts the marquee "animation"
stop_marquee    - stops the marquee "animation"
set_text        - accepts a text input and displays it as a marquee
set_speed       - sets the marquee animation refresh in milliseconds
exit            - terminates the console
```

### 4.2 set_text <string>
* **Syntax:** `set_text <your text here>`
* **Arguments:** One or more words forming the new marquee string (remainder of the line after `set_text `).
* **Description:** Parses everything after the `set_text ` prefix on the same line, stores it in a `std::string` variable in memory, and confirms the save. This text replaces the current marquee content, and is what the band shows: with the marquee stopped the band repaints with the new text as plain art, and with the marquee running the next frame scrolls it. No art is printed below the confirmation line.
* **Behavior:**
    * If no argument is provided (i.e., the line is just `set_text`), print an error:
      ```text
      Error: set_text requires a non-empty string argument.
      ```
    * Otherwise, store the string and print:
      ```text
      Text saved for marquee: <your text here>
      ```

### 4.3 start_marquee
* **Syntax:** `start_marquee`
* **Arguments:** None
* **Description:** Begins the marquee animation using the currently stored text string. The animation runs on its own thread, so the command loop stays responsive and the user can keep typing (Section 9).
* **Output:**
```text
Marquee started. Use stop_marquee to stop it.
```
* **Behavior:**
    * If the marquee is already running, print:
      ```text
      Marquee is already running.
      ```
    * If the font files could not be loaded, print:
      ```text
      Marquee not started. The ASCII-art font did not load; there is nothing to animate.
      ```
    * If standard output is not an interactive terminal (redirected to a file
      or a pipe), print:
      ```text
      Marquee not started. The animation needs an interactive terminal.
      ```
      and leave the marquee stopped, so escape sequences never end up in
      captured output.
    * Otherwise the band starts scrolling and the prompt stays available, so
      the user can keep typing while the marquee plays.

### 4.4 stop_marquee
* **Syntax:** `stop_marquee`
* **Arguments:** None
* **Description:** Stops any running marquee animation and waits for the animation thread to finish, so no frame is drawn after this message.
* **Output:**
```text
Marquee stopped.
```
* **Behavior:** The last frame stays on screen as the final position of the band. This is the normal way to leave the animation; typing it while the marquee runs is expected.

### 4.5 set_speed <milliseconds>
* **Syntax:** `set_speed <integer>`
* **Arguments:** A positive integer representing the refresh interval in milliseconds.
* **Description:** Sets the speed of the marquee animation. Takes effect on the next frame, also when the marquee is already running.
* **Behavior:**
    * If the argument is missing or not a valid positive integer, print:
      ```text
      Error: set_speed requires a positive integer (milliseconds).
      ```
    * Otherwise, store the value and print:
      ```text
      Marquee speed set to <N> ms.
      ```

### 4.6 exit
* **Syntax:** `exit`
* **Arguments:** None
* **Description:** Gracefully terminates the console.
* **Output:**
```text
Terminating console...
```
Then the program exits with code 0.

---

## 5. Implementation Constraints

| Constraint | Requirement |
| :--- | :--- |
| **Language** | C++17 or later |
| **I/O** | `std::cin`, `std::cout` only (no `printf`/`scanf`) |
| **Input parsing** | `std::string`, `std::getline`, `std::istringstream` |
| **Directory Structure** | Application sources live under `src/`: module headers in `src/include/`, implementations in `src/components/`, entry point in `src/main.cpp`. Data assets live under `assets/`. The program resolves each asset by trying `assets/<name>` first, then `<name>` in the working directory; no other subdirectory is searched. |
| **ASCII art font** | Loaded at startup from `assets/ascii_art.txt` (glyph blocks) and `assets/characters.txt` (glyph order), falling back to the working directory - no `figlet`, `ncurses`, or third-party libs permitted. |
| **External libraries**| None permitted |
| **Platform** | Must compile on `g++` (Linux) and MinGW `g++` (Windows) with `-std=c++17` |
| **Character output** | Plain ASCII only (no Unicode escapes or non-ASCII literals) so output renders correctly in `cmd` |

---

## 6. Program State (In-Memory Variables)
The following state must be maintained across commands during a session:

| Variable | Type | Initial Value | Modified By |
| :--- | :--- | :--- | :--- |
| `marqueeText` | `std::string` | `"CSOPESY"` | `set_text` |
| `marqueeSpeed` | `int` (ms) | `200` | `set_speed` |
| `marqueeRunning` | `bool` | `false` | `start_marquee`, `stop_marquee` |

---

## 7. ASCII Art Font Specification
ASCII art glyphs are loaded from two external raw text files that ship in the **`assets/` directory** (CMake also copies them next to the built executable):

* `characters.txt` - the glyph order, one character per line.
* `ascii_art.txt` - the raw glyph blocks, stacked vertically in the same order as `characters.txt`.

The program parses both files once at startup to build an in-memory lookup table.

### 7.1 Character Set
* **Letters:** A-Z (case-insensitive: lowercase input is normalized to uppercase before lookup)
* **Digits:** 0-9
* **Punctuation:** `!`, `?`, `.`, `,`, `-`, `_`, `:`, `;`, `(`, `)`, `'`, `"`
* **Space:** ` ` (not listed in `characters.txt`; rendered as a blank block)

Total: **48 glyphs**, listed in `characters.txt` in this exact order (one character per line):

```text
ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!?.,-_:;()'"
```

The order is data, not code: the program must read it from `characters.txt` rather than hardcode it.

### 7.2 File Format & Layout
`ascii_art.txt` contains raw, un-bordered glyph blocks stacked vertically, one block per entry in `characters.txt`, in the same order. `characters.txt` contains one character per line (a blank line would denote a space glyph).

* **Glyph Height:** Every glyph occupies exactly **6 consecutive lines**.
* **Glyph Width:** File rows are padded to a uniform width (**13 columns**), but at load time each glyph is reduced to its natural ink width (trailing spaces stripped, e.g. `W` keeps 13 columns while `I` keeps 6). Glyphs are joined with a **1-column gap** at render time; spaces and unknown characters use a **4-column** blank block.
* **No Frame Borders:** The file contains raw glyph lines directly (no border stripping or bracket trimming required).
* **Total File Height:** 48 glyphs x 6 lines/glyph = 288 lines.

The glyph height and width are not hardcoded: the height is the number of lines in `ascii_art.txt` divided by the number of entries in `characters.txt`, and the width is the longest ink row found (trailing spaces ignored).

### 7.3 Asset Location & File Access Rule
* The program **must open `ascii_art.txt` and `characters.txt` by name**, resolved by trying `assets/<name>` first and then `<name>` in the current working directory (`"assets/ascii_art.txt"`, then `"ascii_art.txt"`).
* **Do not** search any other directory (e.g., `src/components/ascii_art.txt`). Sources live in `src/include/` and `src/components/`, the two data files live in `assets/`, and CMake copies both data files next to the executable.

### 7.4 In-Memory Representation
```cpp
// One glyph is a list of rows; the map keys characters to glyphs.
std::map<char, std::vector<std::string>> fontMap;
```

**Loading Algorithm:**
1. Open `"characters.txt"` and read one character per line to build the lookup order.
2. Open `"ascii_art.txt"` and read all lines.
3. Compute `height = artLines.size() / order.size()` and `width =` the longest ink row (trailing spaces ignored).
4. For each character, take the next `height` lines, strip trailing spaces from each row, pad the rows to that glyph's own natural ink width, and map the block to the character key.
5. If a character is missing or unknown during lookup (spaces included), fall back to a blank block of `height` rows and 4 columns.

### 7.5 Rendering
To render a string as ASCII art:
1. Convert the input string to uppercase.
2. For each character in the string, look up its natural-width glyph from `fontMap`.
3. Concatenate the blocks horizontally with a 1-column gap, row by row, across all `height` composite rows.
4. Cut any composite row longer than the console width off at that width instead of wrapping it onto extra lines. The console width is the live window width, re-read on every frame, so resizing the window re-cuts the band immediately. (It is deliberately not taken from `COLUMNS`: the rows are drawn at absolute coordinates, and a width that does not match the window makes the Windows console scroll its window sideways.)
5. Trim trailing spaces from each composite row.
6. Print the composite rows to `std::cout`.

```text
Row 0: [Char1_Row0] + [Char2_Row0] + ...
Row 1: [Char1_Row1] + [Char2_Row1] + ...
...
Row 5: [Char1_Row5] + [Char2_Row5] + ...
```

### 7.6 Scrolling Frames
The animation uses a second rendering of the same text, which is not cut off:

1. Build the composite rows exactly as in Section 7.5, but with no width cut. That block is the **band cell**; its width is the **band width**.
2. Compute the **band period**, the distance after which the band repeats. This is what keeps the text from being visible twice at the same time:
   * `bandWidth >= consoleWidth`: the period is the band width, so the head of the text follows its tail immediately and the text reads as one continuous stream.
   * `bandWidth < consoleWidth`: the period is the band width **plus one console width**, so the copy leaving on the right is completely gone before the copy entering on the left shows up.
3. The **band** is the cell padded to the period with spaces, twice over. A window that starts anywhere inside the first period is therefore always full, and the repeat is seamless.
4. A frame is the window that starts `start` columns into the band and is as wide as the console, with the trailing spaces of every sliced row trimmed. `start` is wrapped into one period; because the band repeats, the wrap is invisible.
5. The text travels **from left to right**: the window walks *backwards* through the band, so every character appears one column further right on each frame. The animation keeps a `travel` counter in columns, uses `start = (period - travel) mod period` for the frame, and advances `travel` by one column per frame, wrapping it at the period.

---

## 8. Sample Session
On launch, and again after every command, the screen holds the band, the pinned
header block, the output of the command that was just run, and the prompt:

```text
[ASCII art representation of "CSOPESY"]        <- the band

Group developer:                               <- the pinned block
Trinidad, Nathan
Singh, Nathaniel
Quilantang, Jann Miro
Saguin, VL Kirsten Camille

Version date: 2026-09-18

Command> _
```

```text
Command> help
help            - displays the commands and its description
start_marquee   - starts the marquee "animation"
stop_marquee    - stops the marquee "animation"
set_text        - accepts a text input and displays it as a marquee
set_speed       - sets the marquee animation refresh in milliseconds
exit            - terminates the console
```
(the reply replaced whatever the previous command had shown)

```text
Command> set_text Operating Systems are fun!
Text saved for marquee: Operating Systems are fun!
```
(the band now shows that text as plain art)

```text
Command> set_speed 150
Marquee speed set to 150 ms.

Command> start_marquee
Marquee started. Use stop_marquee to stop it.
```
(the band now scrolls; the prompt below it stays usable, so the next command
can be typed straight away)

```text
Command> set_text Phase 2
Text saved for marquee: Phase 2
```
(the band scrolls the new text from the next frame on)

```text
Command> stop_marquee
Marquee stopped.

Command> bad_command
Unknown command: bad_command

Command> exit
Terminating console...
```

---

## 9. Marquee Animation
The scrolling marquee is a real animation: it runs on its own thread and redraws the band that the welcome title was printed in.

### 9.1 The Band
* The band is **not** a new block further down the screen. It is the block of
  `height` rows at the very top of the screen (row 1), the same rows the ASCII
  art title of Section 2.1 is printed in, so the text that is already on
  screen is what moves.
* A frame is drawn by saving the cursor position, addressing the band rows,
  erasing each row, writing the new one, and restoring the cursor position.
  Everything below the band - the metadata block, the prompt, the replies, and
  whatever the user types - is never touched by the animation.
* The band always has `height` rows, whether the text is short or long, so the
  rows below it never move.
* The text scrolls from **left to right**: it enters at the left edge, crosses
  the band, leaves at the right edge, and comes back in.
* The band repeats at the band period of Section 7.6, which is chosen so the
  text is never on screen twice: a text shorter than the console is fully gone
  before the next copy enters.

### 9.2 The Screen Layout
The console owns the screen below the band and repaints it after every command,
so the layout is always the same and nothing can be pushed around:

```text
row 1 .. height      the band: the marquee, or the text as plain art
row height+1 ..      the pinned header block: group names and version date
                     the output of the command that was just run
last row             Command> _        <- the user types here
```

* The rows above the band are the animation's, and are never cleared by the
  console. Everything below them is erased and written again on every command.
* Only the output of the **last** command is on screen. The previous commands'
  output is dropped, and so is the terminal's own scrollback, because the area
  is repainted rather than appended to.
* A repaint only happens after a command has been submitted, so a command that
  is half typed is never overwritten; the prompt is repainted and the cursor
  placed at its end.
* While the marquee runs, the band is redrawn by the animation thread and the
  console leaves those rows alone. While it is stopped, the console draws the
  current text as plain art there.

### 9.3 Frame Timing
* One frame per `marqueeSpeed` milliseconds, taken from Section 6.
* Each frame renders the scrolling window of Section 7.6 at the current console width, with the window one column further left than the previous frame, which moves the text one column to the right.
* Changing `set_text` or `set_speed` while the marquee runs takes effect on the
  next frame.

### 9.4 Concurrency and Known Limitations
* The animation runs on its own thread, so the command loop keeps reading:
  the user can type, run `help`, change the text, or change the speed while
  the marquee scrolls, and `stop_marquee` ends it.
* Writes from the animation thread and from the command loop are serialized by
  a single write lock on the output stream, so a frame can never be spliced
  into the middle of a line of text.
* `stop_marquee` (and `exit`) wait for the animation thread to finish before
  printing their confirmation, so no frame is drawn after the last message.
* The animation needs an interactive terminal. When the output is redirected
  to a file or a pipe, `start_marquee` reports that and stays stopped, so
  escape sequences never pollute captured output.
* The band is drawn with plain ASCII escape sequences, so terminal emulators
  with ANSI support (Windows Terminal, ConEmu, `cmd` on Windows 10 1511 and
  later, and any VT100-compatible terminal on Linux) are required. The rows
  below the band are cleared through the console API on Windows, and with an
  erase-to-end-of-screen sequence elsewhere.
* Frames use the **live** window width, not `COLUMNS`. A frame is drawn at
  absolute coordinates, and a row wider than the window does not merely look
  too wide: on the Windows console host it scrolls the window sideways, after
  which every following frame is drawn at a shifted position. Each row is also
  cut to the number of columns that actually fit.
* A frame is written without a trailing newline, so the screen never scrolls
  because of the animation itself.

---

## 10. Future Phases (Out of Scope for Phase 1)
The following features are planned for subsequent phases and should not be implemented now, but the current architecture should not prevent them:

1. Color support via ANSI escape codes.
2. Saving/loading marquee text from a file.
3. Multiple named marquee slots.
4. Drawing the band in a reserved screen region so session output cannot scroll it away.
