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
from an external font. The scrolling animation is a future phase;
`start_marquee` / `stop_marquee` currently only record state. `os_emulator.cpp`
is a separate plain-text variant of the same shell.

* **Language:** compiled as C++17, but only the subset GCC 6.3 supports - see
  section 4.
* **Dependencies:** C++ Standard Library only. No third-party libraries, ever.
* **Output:** plain ASCII only, so it renders correctly in `cmd`.

## 2. Build, run, test

```bat
:: direct build (from the repository root)
g++ -std=c++17 -Wall -Wextra -Wpedantic -Isrc/include ^
    src/main.cpp src/components/console.cpp src/components/marquee.cpp ^
    src/components/font.cpp src/components/ascii_art.cpp ^
    src/components/asset_paths.cpp src/components/text_utils.cpp ^
    -o csopesy.exe

:: CMake
cmake -S . -B build
cmake --build build

:: plain-text variant smoke test
test_os_emulator.bat
```

Run from the repository root or from the build directory. The font files are
resolved from `assets/<name>` first, then `<name>` in the working directory, so
both launch locations work.

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

* **MinGW.org GCC 6.3.0** (`-std=c++17`) - the low bar. This is the compiler
  behind the documented `g++` build command.
* **GCC 15.x** (the CLion bundle).

GCC 6.3 advertises C++17 but lacks much of it. All of the following were
verified to fail or warn on GCC 6.3 in this repository:

| Do not use | GCC 6.3 result |
| --- | --- |
| `if (T x = init; cond)` (if-init statement) | **error** |
| `if constexpr` | **error** |
| structured bindings, `auto [a, b] = ...` | **error** |
| inline variables, `inline int x = 1;` | **error** |
| `[[nodiscard]]`, `[[maybe_unused]]`, `[[fallthrough]]` | **warning** `-Wattributes` |
| `<string_view>`, `<optional>`, `<variant>`, `<filesystem>`, `<any>` | header missing |
| `std::byte`, `std::clamp`, `std::gcd` / `std::lcm`, `std::as_const` | **error** |

Safe on GCC 6.3 (also verified): `auto`, `constexpr`, range-based `for`,
lambdas (including generic and `constexpr` lambdas), `std::move`,
`std::make_unique`, nested namespace definitions (`namespace a::b`),
`<map>`, `<vector>`, `<sstream>`, `<system_error>`.

**Rule of thumb: write C++11/14 code.** The `-std=c++17` flag is a spec
requirement, not permission to use the C++17 library or C++17 attributes. When
in doubt, compile with GCC 6.3 before committing.

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
  * `[[nodiscard]]` and if-init - blocked by GCC 6.3 (section 4).
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

1. Builds warning-free on both compilers (section 5).
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
* Never leave the tree unbuildable or warning-producing on GCC 6.3.
