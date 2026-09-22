# Warnings triage

Review of the CLion inspections and clang-tidy findings collected from the
Phase 1 sources. Each item below is either **adopted** (code changed),
**resolved**, or **not adopted** with the reason.

Tooling used: CLion 2026.2.2 inspections, clang-tidy 23.0.0, GCC 6.3.0
(MinGW.org) and GCC 15.2.0 (CLion bundle).

Re-check after changes:

```bat
g++ -std=c++17 -Wall -Wextra -Wpedantic -Isrc/include ^
    src/main.cpp src/components/*.cpp -o csopesy.exe
```

---

## Toolchain constraint (blocks two categories)

`-std=c++17` is requested, but **MinGW.org GCC 6.3.0 predates two C++17
features**, and `CONTRIBUTING.md` requires a warning-free build on both
toolchains. A minimal probe:

```cpp
[[nodiscard]] int f();
if (int v = f(); v > 0) { /* ... */ }
```

```text
GCC 6.3.0: warning: 'nodiscard' attribute directive ignored [-Wattributes]
           error: expected ')' before ';' token        (if-init)
GCC 15.2.0: clean
```

Consequences:

* **`Variable can be moved to init statement`** - not adopted. The
  if-init-statement form does not compile on GCC 6.3.0.
* **`[[nodiscard]]`** - not adopted. GCC 6.3.0 emits `-Wattributes` on every
  use, so adding it would break the "no warnings" build rule.

> Note: an if-init statement was present in `src/components/asset_paths.cpp`
> (an IDE quick-fix) and broke the GCC 6.3.0 build. It has been reverted to a
> plain `if`.

Adopt both later, in one commit, after the MinGW toolchain is upgraded - or
guard `[[nodiscard]]` behind `__has_cpp_attribute(nodiscard)` if the
attribute is wanted sooner.

---

## Common Practices and Code Improvements

| Warning | Location | Decision |
| --- | --- | --- |
| Member function can be made const | `console.cpp` | **Adopted** - `printHeader`, `printHelp`, and `printPreview` are now `const`. `handleSetText`, `handleSetSpeed`, and `run` mutate `marquee_`, so they stay non-const. |
| Non-explicit converting constructor | `console.h` | **Adopted** - `Console` takes defaults for `in`/`out`, so it was implicitly convertible from `Font`; now marked `explicit`. |
| Parameter can be made const | `ascii_art.cpp` (`maxWidth`), `font.cpp` (`ch`), `marquee.cpp` (`speedMs` x2) | **Not adopted** - top-level `const` on by-value parameters changes nothing for the caller and is discouraged by common style guides (Google, C++ Core Guidelines use plain values). Left deliberately. |
| Variable can be moved to init statement | `ascii_art.cpp` (`width`), `console.cpp` (`value`), `font.cpp` (`it`) | **Not adopted** - see toolchain constraint. |

## Potential Code Quality Issues

| Warning | Location | Decision |
| --- | --- | --- |
| Possibly unused `#include` | `console.cpp` | **Resolved** - `<utility>` was unused; adopting pass-by-value (`std::move`) now uses it. `misc-include-cleaner` reports no unused includes in any file. |

## Redundancies in Code

| Warning | Location | Decision |
| --- | --- | --- |
| Redundant `static_cast` | `ascii_art.cpp` | **Adopted (partly)** - removed the no-op casts on the gap width (`kGlyphGap`, a non-negative constant) and on the `reserve` count (`font.height`, already guaranteed positive by `valid()`). Kept `static_cast<int>(composite.size())` because dropping it reintroduces `-Wsign-compare`, and `static_cast<std::string::size_type>(maxWidth)` to document the guarded non-negativity at the point of resize. |
| Redundant member initializer in constructor init list | `console.cpp` | **Adopted** - removed the explicit `marquee_()`; the default constructor still runs. |

## Static Analysis Tools (clang-tidy)

| Warning | Location | Decision |
| --- | --- | --- |
| Use range-based for loop instead | `ascii_art.cpp` (x2), `console.cpp` | **Adopted** - loops over `upper`, `rows`, and `kDevelopers` are now range-based. |
| Pass by value and use `std::move` | `console.cpp` (`Font`), `marquee.cpp` (`std::string`) | **Adopted** - both constructors take their payload by value and move it into the member. |
| Avoid repeating the return type; use a braced initializer list | `font.cpp` (x2) | **Adopted** - `return Font();` is now `return {};`. |
| Use `auto` when declaring iterators | `font.cpp` | **Adopted** - `const auto it = font.glyphs.find(ch);`. |
| Function `valid` should be marked `[[nodiscard]]` | `font.h` | **Not adopted** - see toolchain constraint. |
| Functions `text` / `speed` / `running` should be marked `[[nodiscard]]` | `marquee.h` | **Not adopted** - see toolchain constraint. |
| Constness of `nested` prevents automatic move | `asset_paths.cpp` | **Adopted** - dropped `const` from the local so `return nested;` moves instead of copies; a comment records why it must stay non-const. |

## Syntax Style

| Warning | Location | Decision |
| --- | --- | --- |
| Type can be replaced with `auto` | `font.cpp` (x2), `text_utils.cpp` (x2) | **Not adopted** - these are `std::vector<...>::size_type` / `std::string::size_type` counters. Naming the type states the index domain and is clearer than `auto`; clang-tidy's `modernize-use-auto` does not flag them either. |

---

## Broader scan (beyond the reported findings)

A wider clang-tidy pass (`performance-*,bugprone-*,misc-*,modernize-*`) was run
so findings are not discovered one at a time. After the `nested` fix there are
**no `performance-*` findings left**. The remaining categories:

| Check | Count | Decision |
| --- | --- | --- |
| `modernize-use-trailing-return-type` | 61 | **Not adopted** - every function would become `auto f() -> T`; the classic form reads better and matches the project style. |
| `bugprone-throwing-static-initialization` | 36 (6 constants, `config.h`) | **Adopted** - `config.h` now uses constant-initialized `constexpr char[]` / `constexpr const char*[]` instead of namespace-scope `std::string` / `std::vector`, so nothing is dynamically initialized before `main` and all 36 findings are gone. Call sites wrap in `std::string(...)` where concatenation is needed. |
| `misc-non-private-member-variables-in-classes` | 16 | **Not adopted** - `Font` is a plain data aggregate by design. |
| `misc-include-cleaner` | 16 | **Not adopted** - it wants every transitively-provided std header included directly (IWYU strictness); each unit includes what it actually uses. |
| `bugprone-exception-escape` (`Font`, `main`) | 3 | **Not adopted** - would require wrapping `main` in `try`/`catch`; a startup failure here has no recovery path. This is the only `bugprone-*` finding left. |
| `misc-const-correctness` (`for (char ch : upper)`) | 1 | **Adopted** - the loop variable is now `const char ch`. |

---

## Verification

* Clean build, zero warnings: GCC 6.3.0 and GCC 15.2.0, `-Wall -Wextra
  -Wpedantic`.
* Adopted clang-tidy checks re-run: no findings remain except the four
  `[[nodiscard]]` ones listed above.
* Behavior unchanged: 108/108 output comparisons against the pre-refactor
  binary (3 command sessions x 18 `COLUMNS` states) on both toolchains, plus
  9/9 CMake build-directory runs and the `test_os_emulator.bat` smoke test.
* `bugprone-throwing-static-initialization`: 36 -> 0 after the `config.h`
  conversion. The only remaining `bugprone-*` findings are the three declined
  `bugprone-exception-escape` cases.
