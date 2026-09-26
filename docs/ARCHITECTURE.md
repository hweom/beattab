# Architecture and decisions

## Shared core

`model.hpp/model.cpp` → `format.cpp` → `view.hpp/view.cpp`

- Model: sections, bars, rational positions, timed event variants, arrangements.
- Format: text to model, diagnostics, deterministic model to text.
- Layout: viewport and metrics to page/cell rectangles. No SDL or device driver.
- Renderer: layout/model to a packed 1-bit Canvas (48,000 bytes at 800×480).
- Application: portable timeline state, actions, touch hit testing, playback clock,
  and explicit Display requests. The 800×480 application viewport is the initial
  product profile; the layout and raster implementations accept other dimensions.

The core uses C++17 and the standard library, no exceptions or RTTI APIs, and
compiles without SDL. C++ matches the current CrossPoint/CrossPlay/FreeInk ecosystem;
SDL is only the host platform shell. Existing projects demonstrate this approach,
including native and Emscripten simulator builds. We do not vendor or copy firmware
code or fonts. This is an independent implementation.

The initial fixed-size bitmap font makes snapshots deterministic and avoids OS
font differences. It needs replacement/evaluation for real performance reading.
Typography is not an argument for selecting an incompatible application language.

## Host adapters

- `library`: std::filesystem scanning, file reads, title/artist index, diagnostics.
  It is host-only. Future SD card code should supply streams/text to the same parser.
- `eink`: host-only optical simulation, refresh latency and coalescing. It owns the
  active target plus one pending frame, keeping the newest request. Pending full
  refresh intent survives subsequent partial requests.
- `main`: SDL window, schematic case, keyboard/mouse translation and lab controls.
  SDL never owns song timing or decides pagination. A steady performance clock
  supplies elapsed seconds. Panel frames are requested on application changes;
  lab chrome may redraw at desktop speed without requesting panel refreshes.

A hardware Display adapter must implement asynchronous BUSY handling and upload
packed bits, translating full/partial requests to the selected panel driver.
The current interface passes a borrowed frame; the adapter must copy it if it
retains it. Gray optical buffers (384 kB plus host texture) are simulator-only.
The SDK integration and flashable firmware are explicitly not part of this milestone.

## Constraints and tradeoffs

- Quarter-note BPM is unambiguous in compound meters. A future tempo-unit field
  could express dotted-quarter tempo without silently changing existing songs.
- Exact arithmetic is used for positions, not wall-clock seconds. Bounded parsed
  fractions prevent arithmetic overflow in current operations. Constructing arbitrary
  huge Rational values programmatically is not supported; the constructor's zero
  denominator fallback is internal defensive behavior, not valid file syntax.
- Repeat expansion keeps references, not copies of bars. It is bounded but still
  materialized; firmware should revisit limits and heap allocation.
- Full refresh on page changes; partial on same-page state changes. Bar highlighting
  changes only once per bar. Fast tempos can outrun the simulated screen: BUSY status
  exposes the delay and the latest-frame queue prevents an unbounded backlog.
- Lyrics below are the default. Inside and timeline-aligned modes are evaluation
  variants. All use identical musical coordinates and bar boundaries.
- Library state is not stored in canonical songs. Future favorites/setlists/recent
  history should live in the ignored `state/` directory, keyed by stable song IDs.
- Invalid files are excluded and reported, while valid files remain usable. F5
  rescans and resets playback; file watching and preserving position are deferred.

## Open evaluation questions

1. 2, 3 or 4 bars per row at actual playing distance? Landscape mounting angle?
2. How should dense chord changes and long lyric phrases overflow? Current behavior
   shrinks chord labels and clips text with ellipses, never stretches bar time.
3. Should sections start a fresh row/page, versus filling available cells?
4. Prefer inverted bar, marker strip, or heavier border? Currently marker + border.
5. Is partial-refresh bar latency acceptable? Should a predictive page advance
   lead the downbeat, and how should that lead be represented?
6. Which physical panel/controller variant does the target unit use? Calibrate
   refresh and enclosure occlusion on it before making hardware-fidelity claims.
7. Do lyrics need ranges across bars? Should chord sustain persist across barlines?
8. What parser/heap budgets and file streaming strategy fit alongside the SDK?

Stop here for interactive feedback. Do not implement Phase 2 or the remaining
library features until the notation and small-screen layout have been evaluated.
