# AGENTS.md

Rules for everyone who edits this repository - human or AI. Read this before
changing code. It exists mainly to keep the build **warning-free on both
supported compilers**.

Companion documents:

* [README.md](README.md) - what the program does and how to run it.
* [CONTRIBUTING.md](CONTRIBUTING.md) - branches, commits, review process.
* [SPECIFICATIONS.md](SPECIFICATIONS.md) - the required behavior.
* [warnings.md](warnings.md) - every static-analysis finding, and why the
  declined ones stay declined.

---

## 1. What this project is

CSOPESY Phase 1: an interactive console that renders marquee text as ASCII art
from an external font, and scrolls it. `start_marquee` runs the animation on
its own thread, redrawing the welcome title rows at the top of the screen, so
the command loop stays usable while the band scrolls; `stop_marquee` ends it.
`os_emulator.cpp` is a separate plain-text variant of the same shell.

* **Language:** C++17, built with the mingw-w64 toolchains in section 4. The
  code stays in a conservative subset by choice, not because the compiler
  requires it.
* **Dependencies:** C++ Standard Library only. No third-party libraries, ever.
  The animation needs `<thread>` and `<mutex>`, so the build passes `-pthread`
  (required on Linux/macOS, accepted and ignored by MinGW).
* **Output:** plain ASCII only, so it renders correctly in `cmd`. The animation
  also emits ASCII escape sequences (cursor address, erase line, save and
  restore cursor); that is what redraws the band in place.

## 2. Build, run, test

```bat
:: direct build (from the repository root)
g++ -std=c++17 -pthread -Wall -Wextra -Wpedantic -Isrc/include ^
    src/main.cpp src/components/console.cpp src/components/marquee.cpp ^
    src/components/font.cpp src/components/ascii_art.cpp ^
    src/components/asset_paths.cpp src/components/terminal.cpp ^
    src/components/text_utils.cpp -o csopesy.exe

:: CMake
cmake -S . -B build
cmake --build build

:: plain-text variant smoke test
test_os_emulator.bat
```

Run from the repository root or from the build directory. The font files are
resolved from `assets/<name>` first, then `<name>` in the working directory, so
both launch locations work.

`g++` on `PATH` must be one of the toolchains in section 4. On this machine the
bare `g++` resolves to MinGW.org GCC 6.3.0 (`C:\MinGW\bin\g++`), which cannot
build the threaded files - use the supported compiler by full path
(`C:\msys64\ucrt64\bin\g++.exe`), and keep `C:\msys64\ucrt64\bin` on `PATH` so
that compiler's own runtime DLLs resolve for `cc1plus` and the built binary. The
CMake build avoids the question: the CLion toolchain is the GCC 15.2 in
section 4.

## 3. Layout and module rules

```text
src/main.cpp              entry point (thin: load font, run console)
src/include/*.h           module headers
src/components/*.cpp      module implementations
assets/                   ascii_art.txt, characters.txt
os_emulator.cpp           separate plain-text variant
```

* `src/main.cpp` stays a thin entry point. Put logic in a module.
* Headers declare, components define. Match the filename to the module.
* Include module headers by **bare name** (`#include "font.h"`); the build adds
  `src/include`. Never write `#include "../include/font.h"`.
* Adding a module: create `src/include/<name>.h` and
  `src/components/<name>.cpp`, then add the `.cpp` to `add_executable(...)` in
  `CMakeLists.txt`.

## 4. Toolchain compatibility - HARD RULES

Every change must build warning-free with **both**:

* **GCC 14.2.0, MSYS2 `ucrt64`** (`C:\msys64\ucrt64\bin\g++.exe`) - the low
  bar.
* **GCC 15.2.0**, the mingw-w64 bundle that ships with CLion
  (`...\CLion 2026.2.2\bin\mingw\bin\g++`).

Both are **mingw-w64** builds with the **posix** thread model, which is what
makes `std::thread` and `std::mutex` available at all. Any `g++` 14 or newer
with that thread model behaves the same way; on Linux/macOS `-pthread` is
required, and the CMake build asks for it through `Threads::Threads`.

**Not supported: MinGW.org GCC 6.3.0** (`C:\MinGW`), the previous low bar. It is
a **win32** thread-model build: its libstdc++ has no `std::thread` /
`std::mutex`, so it cannot compile the animation (`marquee.cpp`, `terminal.cpp`,
`console.cpp`, `main.cpp`), with or without `-pthread`. It also hides `_fileno`
under `-std=c++17` because strict ANSI mode is on. Do not add a workaround for
it - threading is a requirement of the design, not an implementation detail.

Because the low bar is now GCC 14.2, the C++17 library and C++17 attributes are
available. The code still reads conservatively on purpose (`std::move`,
`constexpr`, range-based `for`, explicit signatures), and the features that were
only ever blocked by GCC 6.3 - `if`-init statements, `[[nodiscard]]`,
`std::clamp`, the `<optional>` / `<string_view>` headers - are declined in
`warnings.md` as a *style* decision rather than a toolchain limit. Adopt any of
them in their own commit, with `warnings.md` updated in the same change, instead
of sprinkling them in.

## 5. Zero-warning policy

* `-Wall -Wextra -Wpedantic` must be clean on **both** compilers. A warning is
  a bug.
* Namespace-scope string data must be **constant-initialized**: use
  `constexpr char[]` / `constexpr const char*[]` in `config.h`, never
  `std::string` / `std::vector` (those are dynamically initialized before
  `main`, and a throw there terminates the program). Wrap with
  `std::string(kAssetDir)` where concatenation is needed.
* Static analysis is triaged in `warnings.md`. Adopt patterns that are already
  in place, and do **not** re-litigate the declined checks:
  * `modernize-use-trailing-return-type` - classic signatures are the style.
  * `misc-non-private-member-variables-in-classes` - `Font` is a plain aggregate.
  * `misc-include-cleaner` - transitive standard headers are fine.
  * `bugprone-exception-escape` on `main` - startup failure has no recovery.
  * `[[nodiscard]]` and if-init - no longer blocked (section 4); left declined
    so that adopting them is one deliberate change, not a sprinkle.
* Patterns to keep (they keep clang-tidy clean):
  * `explicit` on constructors callable with one argument.
  * Sink parameters by value, then `std::move` into the member.
  * Range-based `for` instead of index loops.
  * `return {};` instead of `return Type();`.
  * `const auto` for iterators.
  * Member functions that do not mutate state are `const`.
  * A local that is returned by value must **not** be `const` - it blocks the
    implicit move.
  * Keep `static_cast<int>(x.size())` in comparisons to avoid `-Wsign-compare`.
  * Do not add top-level `const` to by-value parameters or `auto` to index
    types; those are deliberately declined (see `warnings.md`).

## 6. Code style

* Doxygen file header with the course/authors block on every source file.
* `std::cin` / `std::cout` only - no `printf` / `scanf`. `Console` takes
  injected `std::istream&` / `std::ostream&` so the loop can be driven in tests.
* One concern per module; keep functions short and named for what they do.
* Plain ASCII in all output strings - no Unicode literals or escapes.
* Keep comments minimal and about intent; document non-obvious decisions (e.g.
  why a local is not `const`).

## 7. Definition of done

1. Builds warning-free on both toolchains (sections 4 and 5).
2. `test_os_emulator.bat` still passes (exit 0).
3. A smoke session covers `help`, `set_text` (short and long), `set_speed`
   (valid and invalid), `start_marquee`, `stop_marquee`, an unknown command,
   and `exit`.
4. Output matches `SPECIFICATIONS.md`. If behavior changed on purpose, update
   `SPECIFICATIONS.md` and `README.md` in the same change.
5. New or changed static-analysis findings are recorded in `warnings.md`.

## 8. Git

Follow `CONTRIBUTING.md`: `<prefix>/<short-topic>` branches and Conventional
Commits (`type(scope): summary`). One logical change per commit. Binaries are
gitignored - never commit `.exe` / object files.

## 9. Never

* Never add a third-party dependency.
* Never hardcode an asset path or move the font data out of `assets/`; always
  use `resolveAssetPath`.
* Never reopen the flat layout - headers live in `src/include/`.
* Never edit `os_emulator.cpp` as part of marquee/console work; it is a
  separate deliverable.
* Never leave the tree unbuildable or warning-producing on either toolchain
  in section 4 (MSYS2 `ucrt64` GCC 14.2, CLion GCC 15.2).
