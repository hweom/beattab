# BeatTab

A guitar-mounted performance reader built around **section → bar → beat**.
Lyrics annotate musical time; whitespace never encodes timing.

Milestone 1 is ready for notation and UI evaluation. It includes a dependency-free
C++17 core, file-backed library, and native SDL2 simulator with an **480 × 800 portrait**
landscape, 1-bit framebuffer for the Xteink X4 Pro. **No firmware is built or flashed.**

## Run

macOS (Xcode command-line tools and Homebrew):

```sh
brew install sdl2
cd ~/code/beattab
make
make test golden
make run
```

Linux (Debian/Ubuntu): `sudo apt install build-essential libsdl2-dev`, then the
same `make` commands. The test suite needs only a C++17 compiler and Make.
Run commands from the repository root.

```sh
./build/beattab --search 'Amber'        # original folk-style example
./build/beattab --search 'tab'          # filter title/artist, then N/B to browse
./build/beattab --library /path/to/songs
make validate                          # scan and report file:line diagnostics
make sanitize                          # address + undefined-behavior checks
make snapshots                         # 36 deterministic PGM renderings in build/
./build/beattab --snapshot build/song.pbm
```

The simulator is a fixed device lab window. The panel is rendered pixel-for-pixel;
your OS display scaling and physical monitor size are not a real device's DPI.
Its surrounding shell is schematic, not a dimensionally certified enclosure.

## Controls

| Input | Action |
| --- | --- |
| Up / Down | Physical page keys: previous / next bar |
| P | Physical power: simulated sleep / wake |
| Space, or tap PLAY/PAUSE on panel | Play / pause |
| R, or tap RESTART | Restart |
| Left / Right | Previous / next section occurrence (desktop shortcut) |
| + / - | Tempo multiplier, 25–200% |
| 2 / 3 / 4 | Compare bars per row |
| L | Lyrics below, inside, or aligned to timeline |
| F / U | Full / partial refresh |
| G | Ghosting: off, 18%, 40% |
| [ / ] | Front-light brightness |
| N / B | Next / previous matching song |
| F5 | Rescan manually edited files; report invalid songs in terminal |
| Escape | Quit |

The two case-side keys and power are clickable. In the chosen landscape
orientation, the upper side key is Next and the lower key is Previous. Click a
bar to seek. The footer provides touch-accessible playback/navigation. Other
keys are explicitly desktop development conveniences, not invented hardware.

## Song example

```text
beattab 1
id "my-song"
title "My song"
artist "Me"
key "Am"
tempo 120
time 4/4
capo 2

section "Verse"
bar
chord 1 "Am"
lyric 1 "Count a steady rhythm"
bar
chord 1 "C"
chord 3 "G"
tab 1 5 3 1
tab 3 6 3 1

play "Verse" 2
```

Beat positions are **1-based**; `5/2` means beat 2½. Tempo is always **quarter
notes per minute**, so a 6/8 bar at 120 lasts 1.5 seconds. Tab events use the same
positions: `tab BEAT STRING FRET DURATION`; string 1 is high E. See
[the full format specification](docs/FORMAT.md).

Add optional `capo 2` to a song's header for a top-right capo pictogram and fret
number. Omit it to hide the indicator; `capo 0` explicitly means no capo. Values
0–24 are supported. Chord shapes and tab frets are not transposed automatically.
Amber Road demonstrates capo 2.

## What's included

- Parser, normalized exact fractions, deterministic serializer, bounded repeat expansion.
- Recursive `.song` scanning, stable IDs, duplicate detection, title/artist search,
  artist/title ordering, and source diagnostics. Song files remain authoritative.
- 13 synthetic fixtures covering all requested musical cases and 3 original examples;
  no copied commercial lyrics or tabs.
- Headless layout, packed monochrome raster primitives, timeline-driven playback,
  current-bar highlighting, touch and button action routing.
- Partial/full refresh requests with BUSY latency, coalescing, ghost retention,
  simulated full-refresh flash, front light, and refresh counters.
- Unit tests, sanitizer checks, and 36 golden framebuffer hashes.

## Evaluation boundaries

This is an exploratory **Milestone 1**, not the full Phase 1 product. Favorites,
setlists, recently played, live filesystem watching, a device song browser, tap
tempo, lyric range syntax, and polished typography remain deferred. Search is a
launch argument; F5 reloads files. There is no database server or network access.

Refresh delays (800 ms full / 160 ms partial) and ghosting are **illustrative**,
not measured device waveforms. The renderer uses black and white only; optical
gray in the simulator represents residual ink, not a verified grayscale mode.
The model accepts UTF-8 text but the initial bitmap font supports ASCII only.
Long/dense content is clipped with ellipses; notation needs evaluation before
adding sophisticated collision handling. Tab durations are retained in the model,
but this first cheat-sheet renderer prints fret onsets, not full rhythmic notation.

[Architecture and decisions](docs/ARCHITECTURE.md) ·
[Hardware research and unresolved questions](docs/HARDWARE.md) ·
[Evaluation checklist](docs/EVALUATION.md)

Lyric placement defaults to **Timeline**, which aligns phrases to their beat
positions (for example, `lyric 1.5 "Enter here"`). Press **L** to compare
left-aligned Below/Inside layouts; those modes do not show timing offsets.
**F5** reloads the song while retaining the selected lyric layout and bars per row.

The simulator opens in portrait with **two bars per row** (four rows / eight
bars per page). Its resizable desktop window scales the panel and touch targets
together; F5 keeps the selected bar count.
