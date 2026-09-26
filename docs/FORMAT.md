# BeatTab song format, experimental version 1

Files are UTF-8, line-oriented `.song` text. Spaces delimit fields; indentation
is cosmetic. Blank lines and `#` comments outside quoted values are ignored.
Unknown commands and trailing tokens are errors. Strings use double quotes and
backslash escaping for embedded quotes/backslashes. Literal line breaks inside
strings are not supported. Source errors report path and 1-based line; final
structural errors report end-of-file. There are no column diagnostics yet.

## Header

First non-comment line: `beattab 1`. Exactly one of each metadata field must
precede all sections:

```
id "stable-human-assigned-id"
title "Display title"
artist "Artist"
key "Am"
tempo 120
time 4/4
```

IDs survive moves, title edits, and serialization. They contain 1–80 lowercase
letters, digits, dash, underscore or dot. The scanner rejects duplicate IDs.
Key is a descriptive string, not transposition logic. Tempo: integer 20–400
quarter notes per minute. Meter numerator: 1–32; denominator: 1,2,4,8,16,32.
Required strings are nonempty and at most 256 bytes.

## Optional capo

Add `capo 2` anywhere in the metadata header (before the first section). This
sets a full-width capo at fret 2 and displays a small neck-and-clamp pictogram
with the fret number in the top-right corner. Valid values are integers 0–24;
`capo 0` explicitly indicates no capo (unclamped neck and 0). Omit the field to
show no indicator. Duplicate or misplaced declarations are errors.

This is a playing instruction: chord labels remain the shapes to play, and tab
frets remain relative to the capo. No automatic transposition is performed;
`key` remains descriptive metadata. Serialization preserves both an omitted
setting and an explicit zero. This optional field is part of experimental v1.

## Sections and bars

`section "Any name"` declares a unique section. `bar` starts the next bar in it.
Each section begins with song-default meter and tempo. Bar changes inherit
within that section:

```
section "Bridge"
bar time=3/4 tempo=90
bar
bar time=6/8 tempo=120
```

`bar length=1` makes a one-beat pickup in the current meter. Length is positive
and no greater than the numerator; its unit is the meter's denominator note.
Length does NOT inherit. In 6/8, `length=3` is three eighth notes. Overrides can
appear in any order; duplicates are invalid. Canonical serialization writes
resolved time, tempo and length on every bar to make playback context explicit.

## Events

Event positions are rational **1-based** beats within the bar; the internal
model stores rational zero-based offsets. Accepted numeric syntax is `N` or
`N/D` (no decimals, no negatives). Input numerator ≤1024, denominator 1–64.
The fraction is normalized, so `6/4` and `3/2` are identical positions.

```
chord 1 "Am"
chord 5/2 "G7"
lyric 1 "Approximately anchored phrase"
note 3 "Let ring"
tab 1 6 0 1/2
tab 3/2 5 2 1
```

- Chord: beat, label (1–16 bytes). Labels are opaque; minor/major case is preserved.
  A chord lasts to the next chord or bar end. No implicit cross-bar sustain in
  version 1: repeat the label in each intended bar. A missing chord is unspecified,
  not automatically a rest.
- Lyric/note: beat, text. Anchors need not be syllable-accurate. They are point
  annotations in v1; the internal Event has a duration for future ranges.
- Tab: beat, string (1–6, high E=1), fret (0–24), positive duration in denominator
  beats. Duration must fit this bar. Simultaneous different strings are allowed.
  Sustains crossing a barline must be split; ties and techniques are deferred.

Events must start before bar end. Two events of the same kind at the same
position are invalid, except tab events on different strings. Mixed event
kinds at one position are valid. Events are sorted by offset, kind, and string
in the parsed model. Canonical serialization preserves this order, escapes
strings, uses reduced fractions, LF newlines, and explicit play order. Comments
and original whitespace are not retained by serialization. Scanning never
rewrites source files.

## Arrangement

```
play "Verse" 2
play "Chorus" 1
play "Verse" 1
```

Each line appends complete section occurrences. Counts are 1–64. If no play lines
exist, sections play once in declaration order. Forward section references are
allowed; missing sections are errors. Navigation distinguishes separate repeated
occurrences even when both refer to the same section. A section starts from its
own resolved bar settings on every repeat.

Limits: 1 MiB per file, 128 sections, 1024 bars per section, 128 events per bar,
16384 expanded bars. These are initial parser guardrails, not verified firmware
memory budgets. Large valid libraries still need streaming work before Phase 2.

## Timing

A bar of rational length L in meter N/D at quarter-note tempo Q lasts
`L × 4/D × 60/Q` seconds. Musical positions and quarter-note lengths are exact
fractions; the playback clock converts durations to double-precision seconds.
Playback consumes elapsed time across multiple bars, preserving overshoot. Pausing
freezes the clock. Seeking resets local elapsed time and retains playing state.
At the final bar end playback stops; Play again restarts the song. Restart retains
the current play/pause state. A global tempo multiplier affects all local tempi.
