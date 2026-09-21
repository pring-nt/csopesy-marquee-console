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
* All console output is plain ASCII so it renders correctly in `cmd`.

See [SPECIFICATIONS.md](SPECIFICATIONS.md) for the full behavior spec and
[CONTRIBUTING.md](CONTRIBUTING.md) for build/run instructions and
contribution conventions.

## Repository layout

All files live side by side in a single flat root directory:

```text
main.cpp          - the entire program (single translation unit)
os_emulator.cpp   - plain-text variant (no ASCII-art conversion)
test_os_emulator.bat - automated smoke test for os_emulator
ascii_art.txt     - raw glyph blocks, 6 rows per glyph, 48 glyphs
characters.txt    - glyph order, one character per line
SPECIFICATIONS.md - behavior and font specification
CONTRIBUTING.md   - conventions and how to build/run
```

The font files must sit next to the executable (the program opens
`ascii_art.txt` and `characters.txt` from the working directory).

## Quick start

```bat
g++ -std=c++17 main.cpp -o csopesy.exe
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
