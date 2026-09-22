# CSOPESY Marquee Console

An interactive console shell (C++17, standard library only) that previews
marquee text as ASCII art. This is the Phase 1 foundation for a future
scrolling marquee animation system.

## Current state

* On launch, the program renders `CSOPESY` as ASCII art, prints the group
  metadata block, and enters a `Command>` read-eval-print loop.
* `set_text` stores a new marquee string and immediately previews it as
  ASCII art. Glyphs render at their natural ink width with a 1-column gap;
  any art row wider than the console is cut off (not wrapped), since the
  future scrolling marquee will reveal the hidden part.
* `start_marquee` / `stop_marquee` are acknowledged stubs; the animation
  itself is not implemented yet.
* `set_speed` stores the refresh interval (ms) for the future animation
  engine. `help` lists commands, `exit` quits.
* The code is split into focused modules under `src/` - console shell,
  marquee state, ASCII-art renderer, font loader, text helpers, asset
  resolver, and config constants - so the animation can be added without
  growing a single translation unit. The scrolling seam is
  `Marquee::start`/`stop` plus the row vector returned by `renderAsciiArt`.
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
  marquee.h               - marquee state and lifecycle API
  text_utils.h            - trim/uppercase/positive-integer API
src/components/           - module implementations (.cpp)
  ascii_art.cpp           - width query, row rendering, printing
  asset_paths.cpp         - assets/ then working-directory lookup
  console.cpp             - header, help table, REPL loop, dispatch
  font.cpp                - glyph parsing and font construction
  marquee.cpp             - marquee text/speed/running state
  text_utils.cpp          - string helper implementations
assets/ascii_art.txt      - raw glyph blocks, 6 rows per glyph, 48 glyphs
assets/characters.txt     - glyph order, one character per line
os_emulator.cpp           - plain-text variant (no ASCII-art conversion)
test_os_emulator.bat      - automated smoke test for os_emulator
SPECIFICATIONS.md         - behavior and font specification
CONTRIBUTING.md           - conventions and how to build/run
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
g++ -std=c++17 -Isrc/include src/main.cpp src/components/console.cpp ^
    src/components/marquee.cpp src/components/font.cpp ^
    src/components/ascii_art.cpp src/components/asset_paths.cpp ^
    src/components/text_utils.cpp -o csopesy.exe
csopesy.exe
```

```text
Command> set_text Operating Systems are fun!
Command> set_speed 150
Command> start_marquee
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
