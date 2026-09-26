# Milestone 1 evaluation

1. `make && make test golden validate`; run `make run`.
2. Browse with N/B, or launch `--search Amber` for the original song.
3. Press Space; observe bar transitions and full refresh when crossing pages.
4. Pause, restart, step bars with Up/Down and section occurrences with Left/Right.
5. Compare 2/3/4 bars per row and L lyric placement modes at guitar-glance distance.
6. Use mouse touch on bars/footer and click the two side keys and power key.
7. Change ghosting, full/partial refresh, brightness, and tempo; observe BUSY/counters.
8. Edit a song and F5 to rescan; invalid files should show path/line diagnostics.
9. Inspect `make snapshots` images for unusual fractional changes and tab onsets.

The 13 numbered fixtures cover simple 4/4, repeated chords, two chords per bar,
fractional beat changes, 3/4, 6/8, pickup, instrumental, sparse lyric anchors, tab,
mixed tab/chords, repeated verse/chorus, and meter/tempo changes. Three original
examples (Amber Road, Harbor Waltz, Copper Wire) supply longer arrangements without
copying commercial lyrics/tabs.

Automated checks cover parsing failures, source locations, canonical round trips,
fractions, bar durations, repeated occurrence navigation, pause/restart/end,
tempo scaling, touch actions, sleep, pagination, bar highlight pixels, display
refresh policy, latency, ghost clearing and latest-frame/full-refresh coalescing.
The deterministic mutation smoke runs under address/undefined-behavior sanitizers.

`make golden` compares 36 framebuffer hashes for 2/3/4 bars × three lyric modes ×
four representative songs. Review generated images before intentionally replacing
`tests/golden.txt` with `build/snapshots.txt`. No GUI is required for these tests.
Native GUI smoke: process startup was checked on macOS; frame layouts were inspected
from generated render images. Physical-device validation remains outstanding.
