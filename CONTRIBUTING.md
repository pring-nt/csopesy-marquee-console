# Contributing to CSOPESY Marquee Console

## Getting started

Prerequisites:

* A C++17 compiler: `g++` (Linux) or MinGW `g++` (Windows).
* Optional: CMake 3.x and your IDE's toolchain (the repo ships a
  `CMakeLists.txt` for CLion / IDE builds).
* No external libraries are permitted; the program uses only the C++
  Standard Library.

## How to run the program

1. Clone the repository and enter it:

   ```bat
   git clone <repo-url>
   cd csopesy-marquee-console
   ```

2. Build (pick one):

   ```bat
   g++ -std=c++17 -Isrc/include src/main.cpp src/components/console.cpp ^
       src/components/marquee.cpp src/components/font.cpp ^
       src/components/ascii_art.cpp src/components/asset_paths.cpp ^
       src/components/text_utils.cpp -o csopesy.exe
   ```

   or with CMake:

   ```bat
   cmake -S . -B build
   cmake --build build
   ```

3. Run from the repository root, or from a build directory after CMake has
   copied the fonts next to the executable. The program looks for each font
   file in `assets/` first, then in the working directory:

   ```bat
   csopesy.exe
   ```

4. Try a session:

   ```text
   Command> help
   Command> set_text Operating Systems are fun!
   Command> set_speed 150
   Command> start_marquee
   Command> stop_marquee
   Command> exit
   ```

   To simulate a narrow console (e.g. to check art truncation), set the
   `COLUMNS` environment variable before running:

   ```bat
   set COLUMNS=60 && csopesy.exe
   ```

## Branching conventions

* `main` always builds and passes a smoke session. Never push directly.
* Create short-lived branches from `main`. The prefix mirrors the commit
  type used for the change:

  | Prefix      | Meaning                                | Example                |
  | ----------- | -------------------------------------- | ---------------------- |
  | `feature/`  | new behavior (`feat` commits)          | `feature/scroll-anim`  |
  | `fix/`      | bug fixes (`fix` commits)              | `fix/glyph-spacing`    |
  | `docs/`     | spec/README/CONTRIBUTING only          | `docs/readme`          |
  | `style/`    | formatting with no logic change        | `style/clang-format`   |
  | `refactor/` | code restructuring, no behavior change | `refactor/font-load`   |
  | `test/`     | adding or updating tests               | `test/marquee-smoke`   |
  | `chore/`    | build, tooling, housekeeping           | `chore/cmake-tidy`     |

* Keep branches focused: one concern per branch, rebase onto `main`
  before opening a pull request. Branch scope after the prefix is
  lowercase with hyphens (`<prefix>/<short-topic>`).

## Commit conventions

We follow Conventional Commits: `type(scope): short summary`.

| Type       | Meaning                                | Example                                                |
| ---------- | -------------------------------------- | ------------------------------------------------------ |
| `feat`     | new behavior or feature                | `feat(marquee): add horizontal scroll animation`       |
| `fix`      | bug fix                                | `fix(font): use natural glyph widths with 1-col gap`   |
| `docs`     | documentation only                     | `docs(spec): document art truncation at console width` |
| `style`    | formatting, no logic change            | `style(header): align welcome banner spacing`          |
| `refactor` | code restructuring, no behavior change | `refactor(font): simplify glyph width computation`     |
| `test`     | adding or updating tests               | `test(smoke): cover long-text truncation`              |
| `chore`    | build, tooling, housekeeping           | `chore(build): tidy cmake post-build copy`             |

* Rules:
  * Use lowercase, imperative mood (`add`, not `added`).
  * Keep the subject line at most 72 characters.
  * One logical change per commit; split unrelated edits.
  * Reference issues where applicable (`Refs #12`).

## Code style

> Before writing code, read [AGENTS.md](AGENTS.md). It holds the hard rules:
> the GCC 6.3 compatibility limits (no if-init statements, no `[[nodiscard]]`),
> the zero-warning policy, and the module conventions.

* C++17, standard library only (`std::cin` / `std::cout`, no
  `printf` / third-party libs).
* Keep the module layout: headers live in `src/include/`, implementations
  in `src/components/`, and the entry point is `src/main.cpp`. Add a new
  concern as a `src/include/<name>.h` + `src/components/<name>.cpp` pair
  and register the `.cpp` in `CMakeLists.txt`; do not re-grow `main.cpp`.
* Include module headers by bare name (`#include "font.h"`); the build
  adds `src/include` to the search path, so do not write relative paths
  like `#include "../include/font.h"`.
* Open font data only through `resolveAssetPath` (it checks `assets/`
  first, then the working directory); never hardcode a nested path.
* Console output must stay plain ASCII (no Unicode escapes or
  non-ASCII literals) so it renders correctly in `cmd`.
* Document behavior with Doxygen-style comments and keep
  `SPECIFICATIONS.md` in sync with `main.cpp`.

## Pull request process

1. Open a PR against `main` with a brief description and testing notes.
2. Confirm: clean `g++ -std=c++17` build with no warnings, plus a smoke
   session covering `help`, `set_text` (short and long text),
   `set_speed` (valid and invalid), `start_marquee`, `stop_marquee`,
   an unknown command, and `exit`.
3. Update `SPECIFICATIONS.md` and/or `README.md` if behavior changed.
4. Request one review; address feedback before merging.

## Reporting issues

Include the exact command sequence, the full console output (or a
screenshot for layout bugs), your console width, compiler version, and
the expected behavior per `SPECIFICATIONS.md`.
