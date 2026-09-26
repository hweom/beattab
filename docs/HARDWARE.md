# Xteink X4 Pro research

Checked 2026-09-26 against public firmware implementations. These references can
change; they are evidence for the initial profile, not a substitute for probing
the user's physical device. X4 Pro must not be confused with the ESP32-C3 X4.

| Characteristic | Evidence and initial decision |
| --- | --- |
| Panel | 800×480, approximately 4.26-inch diagonal; use landscape 800×480 pixels, 5:3 aspect |
| MCU | ESP32-S3; CrossPlay selects esp32-s3-devkitc1-n16r8 |
| Memory/storage | 16 MB flash, 8 MB PSRAM in the build profile; SDK/application heap budget still to measure |
| Buttons | Two page keys (logical Up GPIO0, Down GPIO7), plus power GPIO3; no front Back/Confirm row |
| Touch | GT911 capacitive touch; simulator maps mouse coordinates to panel pixels |
| Front light | Dual warm/cool PWM channels; milestone simulates brightness only |
| SD | MicroSD over native one-bit SDMMC; filesystem host adapter kept separate |
| Wi-Fi | ESP32-S3 802.11 b/g/n; existing firmware supports networking; not used in M1 |
| Panel variants | SSD1677 / UC8179 / UC8279 reported across production runs; eventual firmware must detect rather than assume |
| Refresh | Full and partial paths exist; simulator uses explicit asynchronous requests |
| Grayscale | M1 deliberately uses a 1-bit path. No verified grayscale mode/level claim; ghost gray is an optical simulation |

## Primary implementation references

- [CrossPlay README](https://github.com/ma-r-s/crossplay): native SDL2 simulator and
  shared C++/Emscripten architecture, 800×480/touch device scope.
- [CrossPlay platformio.ini](https://github.com/ma-r-s/crossplay/blob/xteink/platformio.ini):
  Arduino/PioArduino, X4 Pro S3 memory profile, PSRAM, SDMMC and front-light support.
- [CrossPlay buttons](https://github.com/ma-r-s/crossplay/blob/xteink/docs/buttons.md):
  actual board pin map and on-hardware landscape orientation. Upper side key is
  Down/Next and lower side key is Up/Previous when rotated counter-clockwise.
- [Draftling hardware](https://github.com/clackups/draftling/blob/main/HARDWARE.md):
  tested X4 Pro panel variants, GT911, SDMMC, light and variable enclosure occlusion.
- [CrossPoint X4 Pro discussion](https://github.com/crosspoint-reader/crosspoint-reader/discussions/2842):
  upstream X4 Pro bring-up.
- [FreeInk SDK](https://github.com/Free-Ink/freeink-sdk): likely future driver integration.

## Fidelity limits

The screen resolution/aspect and hardware action inventory follow the sources.
The shell geometry is schematic, with a 20-pixel content margin. Actual physical
case dimensions, glass occlusion, touch rotation, brightness/temperature curves,
grayscale waveforms and partial update artifacts need measurement on the target
unit. We have not verified them on hardware.

The current **800 ms full / 160 ms partial** refresh delays are editable fields in
`EInk`, chosen to make constraints visible, **not manufacturer specifications**.
Ghosting uses a weighted previous image and full refresh clears it. This does not
model voltage waveforms, temperature, aging, panel-specific grayscale, controller
alignment rules or power usage. Power input pauses the app and displays sleep;
it does not emulate electrical shutdown or a long-press sequence.

C++/SDL was selected after reviewing these implementations. Phase 2 should select
and pin a specific SDK revision, measure memory, and implement input/display/storage
adapters; it should not replace the core with an unrelated firmware application.
