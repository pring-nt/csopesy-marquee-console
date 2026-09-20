# CSOPESY - Console CLI Specifications

**Group Developers:** Trinidad, Nathan, Singh, Nathaniel, Quilantang, Jann Miro, Saguin, Kirsten Camille  
**Version Date:** 2026-09-18  
**Language:** C++ (Standard: C++17 or later)

---

## 1. Overview
CSOPESY is a console-based command-line interface (CLI) written in C++. This document specifies the behavior of the Phase 1 implementation: a minimal interactive shell that lays the groundwork for a future marquee animation system.

The program must compile and run as a single executable with no external runtime dependencies beyond the C++ Standard Library.

---

## 2. Program Entry & Welcome Header
Upon launch, the program shall print a styled welcome header before entering the command loop.

### 2.1 ASCII Art Title
The string `"CSOPESY"` must be rendered in ASCII art text using the font loaded from `ascii_art.txt` - no external libraries (e.g., `figlet`) are permitted. The ASCII art is constructed by mapping characters to the font lookup table loaded at startup. The glyph set and file format are described in Section 7.

> **Note:** The ASCII art title is what will eventually scroll as the marquee text. `set_text` replaces this string at runtime.

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
1. Print `Command> ` (with a trailing space, no newline before it).
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
* **Description:** Parses everything after the `set_text ` prefix on the same line, stores it in a `std::string` variable in memory, and confirms the save. This text replaces the current marquee content.  
  Additionally, the stored string is rendered as ASCII art (using the glyph font loaded from `ascii_art.txt`; see Section 7) and printed to the console immediately, showing the user how the marquee text will look when animated.
* **Behavior:**
    * If no argument is provided (i.e., the line is just `set_text`), print an error:
      ```text
      Error: set_text requires a non-empty string argument.
      ```
    * Otherwise, store the string and print:
      ```text
      Text saved for marquee: <your text here>
      ```
      Then render the ASCII art version of the text below the confirmation line.
      If a rendered row is wider than the console, it is cut off at the console
      width instead of wrapped onto extra lines (see Section 7.5); the future
      scrolling marquee will reveal the hidden part.

### 4.3 start_marquee
* **Syntax:** `start_marquee`
* **Arguments:** None
* **Description:** Begins the marquee animation using the currently stored text string. In Phase 1, this command is recognized and acknowledged but the animation itself is a stub.
* **Phase 1 output:**
```text
Marquee started. (Animation stub - not yet implemented)
```

### 4.4 stop_marquee
* **Syntax:** `stop_marquee`
* **Arguments:** None
* **Description:** Stops any running marquee animation. In Phase 1, this command is recognized and acknowledged but the animation itself is a stub.
* **Phase 1 output:**
```text
Marquee stopped. (Animation stub - not yet implemented)
```

### 4.5 set_speed <milliseconds>
* **Syntax:** `set_speed <integer>`
* **Arguments:** A positive integer representing the refresh interval in milliseconds.
* **Description:** Sets the speed of the marquee animation. Stored in memory for future use by the marquee engine.
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
| **Directory Structure** | All source files (e.g., `main.cpp`) and data assets (`ascii_art.txt`, `characters.txt`) **must reside in the same flat root directory**. Executables and code must access files directly without checking subdirectories or relative paths like `src/` or `assets/`. |
| **ASCII art font** | Loaded directly from `ascii_art.txt` (glyph blocks) and `characters.txt` (glyph order) in the working directory at startup - no `figlet`, `ncurses`, or third-party libs permitted. |
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
ASCII art glyphs are loaded from two external raw text files located in the **same directory** as `main.cpp`:

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

### 7.3 Working Directory & File Access Rule
* The program **must open `ascii_art.txt` and `characters.txt` directly** from the current working directory (`"ascii_art.txt"`, `"characters.txt"`).
* **Do not** look inside nested folders (e.g., `assets/ascii_art.txt` or `src/ascii_art.txt`). All source code files (`main.cpp`, headers) and both data files must remain side-by-side in the same directory.

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
4. Cut any composite row longer than the console width off at that width instead of wrapping it onto extra lines. The console width comes from the `COLUMNS` environment variable when valid, otherwise from the live console size, otherwise 80 columns.
5. Trim trailing spaces from each composite row.
6. Print the composite rows to `std::cout`.

```text
Row 0: [Char1_Row0] + [Char2_Row0] + ...
Row 1: [Char1_Row1] + [Char2_Row1] + ...
...
Row 5: [Char1_Row5] + [Char2_Row5] + ...
```

---

## 8. Sample Session
```text
[ASCII art representation of "CSOPESY"]

Group developer:
Trinidad, Nathan
Singh, Nathaniel
Quilantang, Jann Miro
Saguin, Kirsten Camille

Version date: 2026-09-18

Command> help
help            - displays the commands and its description
start_marquee   - starts the marquee "animation"
stop_marquee    - stops the marquee "animation"
set_text        - accepts a text input and displays it as a marquee
set_speed       - sets the marquee animation refresh in milliseconds
exit            - terminates the console

Command> set_text Operating Systems are fun!
Text saved for marquee: Operating Systems are fun!
[ASCII art representation of "Operating Systems are fun!"]

Command> set_speed 150
Marquee speed set to 150 ms.

Command> start_marquee
Marquee started. (Animation stub - not yet implemented)

Command> stop_marquee
Marquee stopped. (Animation stub - not yet implemented)

Command> bad_command
Unknown command: bad_command

Command> exit
Terminating console...
```

---

## 9. Future Phases (Out of Scope for Phase 1)
The following features are planned for subsequent phases and should not be implemented now, but the Phase 1 architecture should not prevent them:

1. Real-time scrolling marquee animation (horizontal ticker) using terminal cursor control.
2. Multi-threaded marquee (animation thread + input thread running concurrently).
3. Color support via ANSI escape codes.
4. Saving/loading marquee text from a file.
5. Multiple named marquee slots.
