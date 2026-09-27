# CSOPESY Marquee Console

An interactive console shell (C++17, standard library only) that scrolls
marquee text as ASCII art. The welcome title is the marquee band: `start_marquee`
makes it scroll in place, while the command loop stays responsive.

## Current state

* On launch, the program prints the group metadata block and enters a
  `Command>` read-eval-print loop. The `CSOPESY` title at the very top is the
  marquee band: `start_marquee` scrolls text through those rows.
* The console owns the screen **below** the band and repaints it after every
  command: header block, the output of the command you just ran, then the
  prompt. The layout never drifts, nothing is overwritten, and there is no
  scrollback to scroll back into - the previous command's output is dropped.
* `set_text` stores a new marquee string and confirms it; the band at the top
  of the screen then shows that text (as plain art when the marquee is stopped,
  scrolling when it runs). Glyphs render at their natural ink width with a
  1-column gap, and any art row wider than the console is cut off (not
  wrapped), since the scrolling marquee reveals the hidden part.
* `start_marquee` scrolls the stored text **from left to right** through **the
  title rows at the top of the screen** - the art that is already on screen
  moves, nothing is printed further down - one column per frame. The text is
  never on screen twice: when it is shorter than the console it leaves at the
  right edge before the next copy enters at the left one. The animation runs on
  its own thread, so you can keep typing: `set_text`, `set_speed`, and `help`
  all work while it plays, and `stop_marquee` ends it. `set_speed` sets the
  refresh interval (ms) between frames.
* The animation needs an interactive terminal; when the output is redirected
  to a file or a pipe, `start_marquee` says so and stays stopped instead of
  writing escape sequences into your capture. The rows below the band are
  cleared and repainted after every command, so the band keeps the top rows no
  matter how many commands you type.
* `help` lists commands, `exit` quits.
* The code is split into focused modules under `src/` - console shell,
  marquee state and animation, ASCII-art renderer, font loader, text helpers,
  asset resolver, terminal cursor control, and config constants - so nothing
  lives in a single translation unit. The seam is `Marquee::start`/`stop`,
  the row vector returned by `renderAsciiArt`/`renderScrollingFrame`, and the
  band drawing in `terminal.cpp`.
* All console output is plain ASCII so it renders correctly in `cmd`.

See [SPECIFICATIONS.md](SPECIFICATIONS.md) for the full behavior spec and
[CONTRIBUTING.md](CONTRIBUTING.md) for build/run instructions and
contribution conventions.

## Repository layout

Application sources live under `src/`, one module per responsibility; the
font data lives under `assets/`:

```text
src/main.cpp              - entry point: load font, run console
src/include/              - module headers (.h)
  ascii_art.h             - console-width query and ASCII-art rendering API
  asset_paths.h           - font-asset location API
  config.h                - defaults, version, developers, layout constants
  console.h               - console shell API
  font.h                  - glyph structures and font-loading API
  marquee.h               - marquee state, lifecycle, and animation API
  terminal.h              - cursor control, output locking, screen clearing
  text_utils.h            - trim/uppercase/positive-integer API
src/components/           - module implementations (.cpp)
  ascii_art.cpp           - width query, row rendering, band measurement
  asset_paths.cpp         - assets/ then working-directory lookup
  console.cpp             - screen repaint, header block, REPL loop, dispatch
  font.cpp                - glyph parsing and font construction
  marquee.cpp             - marquee state and the scrolling frame loop
  terminal.cpp            - band drawing, isatty/VT handling, screen clearing
  text_utils.cpp          - string helper implementations
assets/ascii_art.txt      - raw glyph blocks, 6 rows per glyph, 48 glyphs
assets/characters.txt     - glyph order, one character per line
os_emulator.cpp           - plain-text variant (no ASCII-art conversion)
test_os_emulator.bat      - automated smoke test for os_emulator
SPECIFICATIONS.md         - behavior and font specification
CONTRIBUTING.md           - conventions and how to build/run
AGENTS.md                 - hard rules for contributors and AI agents
```

Each module pairs a header in `src/include/` with its implementation in
`src/components/`; includes are always by bare name (`#include "font.h"`),
so the only include path the build needs is `src/include`.

The console resolves each font file by trying `assets/<name>` first and
then `<name>` in the working directory, so it runs both from the repository
root and from a build directory (CMake copies the fonts next to the
executable).

## Quick start

```bat
g++ -std=c++17 -pthread -Isrc/include src/main.cpp src/components/console.cpp ^
    src/components/marquee.cpp src/components/font.cpp ^
    src/components/ascii_art.cpp src/components/asset_paths.cpp ^
    src/components/terminal.cpp src/components/text_utils.cpp -o csopesy.exe
csopesy.exe
```

`-pthread` is only needed where the thread library is separate (Linux, macOS);
MinGW accepts it as well.

```text
Command> set_text Operating Systems are fun!
Command> set_speed 150
Command> start_marquee
Command> stop_marquee
Command> exit
```

## Automated testing

`test_os_emulator.bat` runs a full smoke pass over `os_emulator.exe`
(no font files needed) and doubles as documentation of every command:

```bat
test_os_emulator.bat
```

It covers `help`, `set_text` (plain words, multi-word, punctuation and
numbers, `&` input, missing and spaces-only arguments), `set_speed`
(valid values plus missing, zero, negative, non-numeric, float, trailing
characters, and overflow inputs), `start_marquee` / `stop_marquee`,
unknown commands, case sensitivity (`HELP` is rejected), padded
whitespace (`   help    ` still works), EOF without `exit`, and a full
sample session. If `os_emulator.exe` is missing, the script builds it
with `g++ -std=c++17 os_emulator.cpp -o os_emulator.exe` first.

To add a case, copy any `TEST` block: pipe command lines into the exe
with `(echo <command> & echo exit) | os_emulator.exe`. Escape
`& | < > ^` as `^& ^| ^< ^> ^^` and write a literal `%` as `%%`.
Never put an odd number of `"` on one piped `(echo ...)` line - an
unbalanced quote swallows the closing paren/pipe and the rest of the
script stops; test `"` input by typing it manually.

## Group developers

Trinidad, Nathan | Singh, Nathaniel | Quilantang, Jann Miro |
Saguin, VL Kirsten Camille
