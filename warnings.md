# Warnings triage

Review of the CLion inspections and clang-tidy findings collected from the
Phase 1 sources. Each item below is either **adopted** (code changed),
**resolved**, or **not adopted** with the reason.

Tooling used: CLion 2026.2.2 inspections, clang-tidy 23.0.0, GCC 14.2.0
(MSYS2 ucrt64, the low bar) and GCC 15.2.0 (CLion bundle).

Re-check after changes:

```bat
g++ -std=c++17 -Wall -Wextra -Wpedantic -Isrc/include ^
    src/main.cpp src/components/*.cpp -o csopesy.exe
```

---

## Toolchain constraint (lifted)

The earlier revision of this file recorded that `-std=c++17` was requested but
**MinGW.org GCC 6.3.0 predated two C++17 features**, so the if-init statement
and `[[nodiscard]]` were declined. The probe was:

```cpp
[[nodiscard]] int f();
if (int v = f(); v > 0) { /* ... */ }
```

```text
GCC 6.3.0: warning: 'nodiscard' attribute directive ignored [-Wattributes]
           error: expected ')' before ';' token        (if-init)
GCC 15.2.0: clean
```

GCC 6.3.0 is no longer a supported compiler (see the toolchain note at the end of
this file), so both features are available now. They stay **not adopted** only
because the code does not use them today and adopting them is a style change of
its own:

* **`Variable can be moved to init statement`** - not adopted; available now.
* **`[[nodiscard]]`** - not adopted; available now.

Adopt them in one commit, with this file updated in the same change, rather than
one site at a time.

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
| Type can be replaced with `auto` | `font.cpp` (x2), `text_utils.cpp` (x2) counters; `ascii_art.cpp` (`from`, `window`) | **Not adopted** - these are `std::vector<...>::size_type` / `std::string::size_type` counters and window bounds. Naming the type states the index domain and is clearer than `auto`. clang-tidy leaves the plain counters alone but does flag the two cast initializers (`modernize-use-auto`); declined for the same reason. |

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

## Scrolling marquee animation (thread + terminal module)

The animation added `src/include/terminal.h` + `src/components/terminal.cpp`,
a `std::thread` in `Marquee`, and a `std::mutex` in both. New findings and the
decisions taken:

| Warning | Location | Decision |
| --- | --- | --- |
| Non-explicit converting constructor | `terminal.h` (`SyncBuf`) | **Adopted** - a `SyncBuf` is meaningless without the stream buffer it wraps, so the constructor taking `std::streambuf*` is `explicit`. |
| Class has pointer data members but does not override copy | `terminal.h` (`SyncBuf`) | **Adopted** - copying a locking buffer would be meaningless, so the copy constructor and assignment are `= delete` (C++11, which is all the
current code needs). The same applies to `Marquee` (mutex, thread, references)
and to `Console`, which now owns the buffer. |
| Include what you use | `marquee.h` (`<mutex>`, `<thread>`, `<iosfwd>`) | **Adopted** - each unit includes what it uses directly. `<iosfwd>` keeps the header light; the destructor still needs the complete `std::ostream` type, so `terminal.h` includes `<ostream>`. |
| Mutex should be `mutable` | `marquee.h` (`stateMutex_`) | **Adopted** - `isRunning()` and `waitForNextFrame()` are `const` helpers called from the worker, so the lock is `mutable`. |
| Use `std::scoped_lock` / `lock_guard` vs. manual lock/unlock | `marquee.cpp` | **Adopted** - `std::lock_guard` is used in single-scope critical sections; the two cases that need to leave the critical section (`start`, `stop`) scope the lock in a block and join outside it, so the console is never blocked while the thread is joined. |
| Repaint the screen instead of appending output | `console.cpp` | **Adopted** - every command fills `screen_` and the console repaints the area below the band. The alternative (letting output accumulate) is what made the layout drift and what the band used to fight with; the trade-off is no scrollback, which is documented in `SPECIFICATIONS.md` Section 9.2. |
| Keep the band rows out of the console's writes | `console.cpp`, `terminal.cpp` | **Adopted** - `clearBelowBand()` starts one row under the band, so the console can never erase a frame, and `paintBand()` skips the band entirely while the animation thread owns it. |
| Spin-wait / busy loop | `marquee.cpp` (`waitForNextFrame`) | **Adopted with a note** - the wait is cut into `kWakeUpGranularityMs` chunks instead of one long sleep, so `stop_marquee` returns within one chunk even after `set_speed 5000` (measured 46 ms). The chunk was 20 ms and is now 50 ms; see the frame-interval pass below for the measurement that motivated it. |
| Use `std::this_thread::sleep_for` instead of `usleep`/`Sleep` | `terminal.cpp` | **Not adopted** - `<thread>` would be pulled into the wait path and the project keeps its platform code in one place; `Sleep`/`usleep` is the same wait without the extra header, and it is what `usleep` already did. |
| Prefer atomic flag over mutex for `running_` | `marquee.cpp` | **Not adopted** - `running_` sits next to `text_` and `speedMs_`, which need a real lock anyway. One mutex for three fields is simpler than a mutex plus an atomic. |
| `drawBand` should be a member | `terminal.cpp` | **Not adopted** - the module already exposes free functions (`getDisplayWidth`, `printAsciiArt`); keeping the style consistent matters more here. |
| Restore the stream buffer in the destructor | `console.cpp` | **Adopted** - `std::cout` is flushed again during static destruction after `main` returns, so the original buffer is put back; otherwise that flush would talk to a destroyed `SyncBuf`. |
| Address the band with `ESC[1;1H` everywhere | `terminal.cpp` | **Not adopted** - the Windows console host counts rows from the top of the *scrollback*, so the band is addressed at `srWindow.Top`/`srWindow.Left` there. Addressing the buffer top pulled the viewport back to the oldest output on every frame. |
| Frames should use the `COLUMNS`-aware width | `terminal.cpp`, `marquee.cpp` | **Not adopted** - the rows are drawn at absolute coordinates, and a row wider than the window scrolls the Windows console window sideways, after which every later frame is drawn at a shifted position: the band drifted sideways and console text survived to its left. The frames use `liveConsoleWidth()` and every row is cut to the columns that fit. `getDisplayWidth()` and `printAsciiArt()` went with the `set_text` preview; `terminal.cpp` is now the only file with platform code. |

**Toolchain note:** the animation needs `-pthread` on Linux/macOS
(`Threads::Threads` in CMake, documented in the build commands of `README.md`,
`CONTRIBUTING.md`, and `AGENTS.md`). On mingw-w64 with the posix thread model
the flag is accepted and `std::thread` / `std::mutex` come from winpthreads;
a win32-threaded MinGW has no such classes at all (see the toolchain note at the
end of this file). Nothing in the new code uses C++17 library features or
attributes - that is a style choice now, not a compiler limit.

---

---

## Frame interval pass (smoother scroll)

`set_speed` reaches the animation as `Console::handleSetSpeed` ->
`Marquee::setSpeed` -> `speedMs_`, which the animation thread re-reads on every
iteration and hands to `waitForNextFrame`. That path is sound; the default value
was too coarse, and the wait granularity was stretching the interval on top of
it. Both were changed:

| Change | Before | After |
| --- | --- | --- |
| `kDefaultMarqueeSpeed` (`config.h`) | 200 ms per column | 50 ms per column |
| `kWakeUpGranularityMs` (`marquee.cpp`) | 20 ms chunks | 50 ms chunks |

Measured with a counting harness (frames counted from the `ESC[s` save-cursor
sequence in the output, 2 s runs, GCC 15.2.0 build):

| Requested interval | 20 ms chunks | 50 ms chunks |
| --- | --- | --- |
| 50 ms (new default) | 76 ms/frame (26 frames / 2 s) | 60 ms/frame (33 frames / 2 s) |
| 200 ms (old default) | 286 ms/frame (7 frames / 2 s) | 222 ms/frame (9 frames / 2 s) |
| 1000 ms | 1003 ms/frame | 1006 ms/frame |
| `stop_marquee` waits while running | ~16 ms | ~46 ms |

The platform timer tick is about 15 ms, so every chunk rounds up to the next
tick and many small chunks pay that rounding once per chunk - which is why
20 ms chunks turned a requested 50 ms frame into 76 ms. At the default rate a
50 ms chunk is a single `Sleep`, and 46 ms of extra latency in `stop_marquee`
is imperceptible. The scroll went from ~3.4 frames/s (old default, measured
286-311 ms/frame) to ~16.5 frames/s (measured 60 ms/frame). `SPECIFICATIONS.md`
Section 6 carries the new initial value.

## Whole-tree warning pass

The tree was re-scanned after the frame-interval change with the gate flags
(`-Wall -Wextra -Wpedantic`) plus a wider set (`-Wshadow -Wconversion
-Wsign-conversion -Wold-style-cast -Wuseless-cast -Wdouble-promotion
-Wformat=2 -Wnull-dereference -Wduplicated-cond -Wduplicated-branches
-Wlogical-op -Woverloaded-virtual -Wnon-virtual-dtor -Wcast-qual -Wcast-align
-Wzero-as-null-pointer-constant -Wredundant-decls -Wundef -Wswitch-enum
-Wswitch-default -Wmissing-declarations -Wimplicit-fallthrough -Wdeprecated
-Wstrict-overflow=2 -Wctor-dtor-privacy`) and by clang-tidy 23.0.0, on every
source file.

| Warning | Location | Decision |
| --- | --- | --- |
| Unused includes left behind by the preview/width code that moved to `terminal.cpp` | `ascii_art.cpp` (`<cstdlib>`, `<istream>`, `<ostream>`, `<sstream>`, `<windows.h>`, `<sys/ioctl.h>`, `<unistd.h>`) | **Adopted** - the renderer needs none of them now, so it is free of platform code. `#include "font.h"` was added so `lookupGlyph` is included directly. |
| `misc-misplaced-const` | `terminal.cpp` (`const HANDLE handle`, x2) | **Adopted** - `HANDLE` is a typedef for `void*`, so the `const` qualified the pointer, not the handle; the misleading qualifier was removed. |
| `readability-math-missing-parentheses` | `font.cpp` (x2, `i * height + row`) | **Adopted** - written as `(i * height) + row`. |
| `cppcoreguidelines-special-member-functions` | `console.h`, `marquee.h`, `terminal.h` | **Adopted** - the deleted copy operations are now matched by deleted move operations, and `SyncBuf` declares its defaulted destructor, so all three classes spell out the rule of five explicitly. C++11, so nothing here depends on C++17. |
| `cppcoreguidelines-use-default-member-init` | `marquee.h` (`running_`) | **Adopted** - `bool running_ = false;`, matching the `Font` aggregate style; the redundant initializer was dropped from the constructor. |
| `bugprone-throwing-static-initialization` | `os_emulator.cpp` (`kDefaultMarqueeText`, `kVersionDate`, `kDevelopers`) | **Adopted** - the same conversion `config.h` already got: `constexpr char[]` / `constexpr const char*[]` instead of namespace-scope `std::string` / `std::vector`; the now-unused `<vector>` include went with it. The plain-text variant is normally reserved by `AGENTS.md` for separate work, so this is confined to the three constants and belongs in its own commit; a full command session produces byte-identical output before and after. |
| `-Wsign-conversion` (outside the gate) | `ascii_art.cpp` (`rows.reserve(font.height)`, `glyph[row]`), `font.cpp` (`Glyph(font.height, ...)`) | **Not adopted** - `font.height` is an `int` by design (the spec and the `Font` aggregate use `int` for rows and columns) and each of these uses is guarded by `font.valid()`, so the value is known positive. Casting only restates that and contradicts the *Syntax Style* decision to keep named index types. With the flag on, every compiler tried here reports the same three (GCC 14.2.0 ucrt64, GCC 15.2.0, and the dropped GCC 6.3.0), so the finding is the flag's, not the code's. |
| `misc-const-correctness` on `std::ostream& out` | `terminal.cpp` (`drawBand`) | **Not adopted** - `const` on a reference is a no-op; the qualifier clang-tidy asks for is one the language ignores. |
| `bugprone-easily-swappable-parameters` | `ascii_art.cpp` (`renderScrollingFrame`), `terminal.cpp` (`bandOrigin`) | **Not adopted** - the fix is a parameter object (`struct Point`) for two internal helpers with two call sites; the parameter names are already in the signature. |
| `cppcoreguidelines-pro-bounds-avoid-unchecked-container-access` | `ascii_art.cpp`, `font.cpp`, `terminal.cpp` (8 findings) | **Not adopted** - every indexed access sits inside an explicit bounds check (`row < height`, `i < rows.size()`, `period > 0`); the alternative is `at()` with exceptions in a per-frame path. |
| `modernize-use-auto` | `ascii_art.cpp` (`from`, `window`) | **Not adopted** - same reason as the counters in *Syntax Style*. |
| `bugprone-exception-escape` on `~Console` | `console.cpp` | **Not adopted** - the destructor is implicitly `noexcept` on purpose. It joins the animation thread and flushes an injected stream whose exception mask is `goodbit` by default, so there is nothing to catch; a throw at teardown has no recovery path, the same rationale as the declined `main` / `Font` cases. |
| `cppcoreguidelines-avoid-c-arrays`, `pro-bounds-array-to-pointer-decay` | `config.h`, `os_emulator.cpp` | **Not adopted** (and the counts rise) - these are the direct consequence of the adopted constant-initialization fix: `constexpr char[]` is a C array. It is the correct shape for compile-time string data here. |

Documentation was corrected in the same pass: `README.md` described `ascii_art.h` /
`ascii_art.cpp` as owning the console-width query, which has lived in
`terminal.cpp` since the preview moved, and the new default interval is stated
there as well.

### Toolchain note: why GCC 6.3.0 was dropped

The low bar used to be MinGW.org GCC 6.3.0-1 (`C:\MinGW`). That build reports
`Thread model: win32`, and its libstdc++ has no `std::thread` / `std::mutex` at
all - `#include <thread>` resolves to nothing and `std::mutex` is "not a member
of 'std'" - so `marquee.cpp`, `terminal.cpp`, `console.cpp`, `main.cpp`, and
every header that reaches `marquee.h` cannot be compiled by it, `-pthread` or
not (MinGW accepts the flag and ignores it). A `-std=c++17` build there also
hides `_fileno` under strict ANSI, while `-std=gnu++17` does not. The five units
that need no threads (`text_utils.cpp`, `asset_paths.cpp`, `font.cpp`,
`ascii_art.cpp`, `os_emulator.cpp`) do compile warning-free with the gate flags,
which is how the earlier "verified on GCC 6.3" claim was able to look clean - it
could never have covered the threaded units.

The supported toolchains are the **posix**-threaded mingw-w64 builds that are
actually installed and verified here: GCC 14.2.0 (MSYS2 `ucrt64`) as the low bar
and GCC 15.2.0 (CLion bundle). `AGENTS.md` section 4 records this. No warning fix
can make a win32-threaded MinGW.org build compile a threaded program.

## Verification

* Gate flags (`-Wall -Wextra -Wpedantic`) clean on both supported toolchains
  (GCC 14.2.0 MSYS2 `ucrt64` and GCC 15.2.0 CLion bundle) for every source file.
* The wider flag set from the whole-tree pass adds only the three declined
  `-Wsign-conversion` findings - nothing else - on both supported toolchains.
* clang-tidy: the adopted checks are clean. What is left is the declined
  categories above (`modernize-use-trailing-return-type`, `avoid-c-arrays`,
  `misc-include-cleaner`, `readability-identifier-length`,
  `misc-non-private-member-variables-in-classes`,
  `pro-bounds-array-to-pointer-decay`, `modernize-use-scoped-lock`,
  `modernize-use-nodiscard`, `pro-bounds-avoid-unchecked-container-access`),
  the four `[[nodiscard]]` cases, the two `modernize-use-auto` cast
  initializers, the two `bugprone-easily-swappable-parameters` helpers, the one
  `misc-const-correctness` reference, and the four declined
  `bugprone-exception-escape` locations. `bugprone-throwing-static-initialization`
  is 36 -> 3 -> 0 across the two passes.
* Behavior unchanged: a full piped command session (help, short and long
  `set_text`, valid and invalid `set_speed`, `start_marquee`, `stop_marquee`,
  an unknown command, `exit`) and a short session produce byte-identical output
  to a build of the pre-change tree, and `os_emulator.exe` is byte-identical
  across a full command session.
* `test_os_emulator.bat` passes (exit 0).
* Frame rate: default 50 ms, measured 60 ms/frame (33 frames in 2 s, ~16.5
  frames/s) against ~286-311 ms/frame before; `stop_marquee` latency ~46 ms.
